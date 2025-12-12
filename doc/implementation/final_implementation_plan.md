# Toy AddressSanitizer 最终实现计划

## 📋 项目现状分析

### 当前已实现的功能
✅ **内存分配器基础架构**
- `toy_malloc()`: 3页内存布局 [保护页][用户数据][保护页]
- `toy_free()`: 完整内存块释放
- 元数据管理：`add_allocation()`, `find_allocation()`, `remove_allocation()`
- 保护页检测：`find_allocation()` 能识别保护页访问

✅ **信号处理基础**
- `setup_signal_handler()`: 注册SIGSEGV处理器
- `sigsegv_handler()`: 基础信号捕获框架

### 当前存在的问题
❌ **信号处理器不完整**
- 存在伪代码和未实现的函数调用
- 缺少真实的错误报告机制
- 访问类型检测逻辑错误

❌ **缺少调用栈记录**
- `toy_malloc()` 未记录分配时的调用栈
- 信号处理器无法显示分配位置信息

❌ **数据结构不完整**
- `allocation_record` 缺少调用栈存储字段
- 缺少必要的常量定义

## 🎯 最终实现目标

### 完整的ASan错误报告
```
=================================================================
==12345==ERROR: Toy AddressSanitizer: heap-buffer-overflow on address 0x6020000000cd
WRITE of size 1 at 0x6020000000cd thread T0
    #0 0x4009f2 in dangerous_function (/path/test+0x9f2)
    #1 0x400a23 in main (/path/test+0x123)
    #2 0x7f1234567896 in __libc_start_main (libc.so.6+0x21b96)

0x6020000000cd is located 5 bytes to the right of 100-byte region [0x602000000050,0x6020000000b4)
allocated by thread T0 here:
    #0 0x400456 in toy_malloc (/path/test+0x456)
    #1 0x400789 in allocate_100byte (/path/test+0x789)
    #2 0x400a23 in main (/path/test+0x123)
SUMMARY: Toy AddressSanitizer: heap-buffer-overflow (/path/test+0x9f2) in main
=================================================================
```

## 🔧 实现计划

### 阶段1: 数据结构扩展

#### 1.1 修改 `src/toy_asan/toy_asan.h`

**动机**: 支持调用栈记录和错误报告

**修改内容**:
```c
// 添加新的常量定义
#define MAX_ALLOC_BACKTRACE 8
#define MAX_BACKTRACE_FRAMES 16

// 扩展allocation_record结构
struct allocation_record {
    void *base_addr;                              // 现有字段
    void *user_addr;
    size_t user_size;
    void *left_guard;
    void *right_guard;
    bool in_use;
    
    // 新增字段：调用栈记录
    void *alloc_backtrace[MAX_ALLOC_BACKTRACE];     // 分配时调用栈
    int alloc_backtrace_size;                      // 调用栈大小
};

// 添加新的函数声明
void print_call_stack(void);
void print_memory_relation(void *fault_addr, struct allocation_record *rec);
void print_allocation_location(struct allocation_record *rec);
const char *infer_access_type(int si_code);
void forward_to_default_handler(int sig, siginfo_t *info);
```

### 阶段2: 内存分配器增强

#### 2.1 修改 `src/toy_asan/toy_malloc.c`

**动机**: 在分配时记录调用栈，为错误报告提供分配位置信息

**修改内容**:
```c
#include "toy_asan.h"
#include <execinfo.h>  // 新增：backtrace支持

void *toy_malloc(size_t size) {
    // ... 现有分配逻辑保持不变 ...
    
    // 在add_allocation之后添加调用栈记录
    struct allocation_record *rec = find_allocation_by_user_addr(user_addr);
    if (rec) {
        // 关键：记录分配时的调用栈
        rec->alloc_backtrace_size = backtrace(rec->alloc_backtrace, MAX_ALLOC_BACKTRACE);
        
        // 调试输出（可选）
        if (rec->alloc_backtrace_size > 0) {
            printf("DEBUG: Allocation stack recorded with %d frames\n", rec->alloc_backtrace_size);
        }
    }
    
    return user_addr;
}
```

**关键点**:
- 在内存分配完成后立即记录调用栈
- 使用GNU的`backtrace()`函数
- 保存完整的分配调用链（包括toy_malloc本身）

### 阶段3: 信号处理器完整实现

#### 3.1 重写 `src/toy_asan/signal_handler.c`

**动机**: 实现完整的ASan风格错误报告，包括调用栈、内存关系、分配位置

**修改内容**:

```c
#include "toy_asan.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // getpid()
#include <execinfo.h>    // backtrace, backtrace_symbols

/**
 * @brief 从si_code推断访问类型
 */
const char *infer_access_type(int si_code) {
    switch (si_code) {
        case SEGV_ACCERR:
            return "WRITE";
        case SEGV_ACCADR:
            return "READ";
        default:
            return "WRITE";
    }
}

/**
 * @brief 打印当前调用栈（错误发生时）
 */
void print_call_stack(void) {
    void *buffer[MAX_BACKTRACE_FRAMES];
    int frames = backtrace(buffer, MAX_BACKTRACE_FRAMES);
    char **symbols = backtrace_symbols(buffer, frames);
    
    // 跳过信号处理器本身，从第1帧开始
    for (int i = 1; i < frames; i++) {
        printf("    #%d %p\n", i-1, buffer[i]);
    }
    
    free(symbols);
}

/**
 * @brief 打印内存位置关系
 */
void print_memory_relation(void *fault_addr, struct allocation_record *rec) {
    bool is_left_overflow = (fault_addr < rec->user_addr);
    const char *direction = is_left_overflow ? "left" : "right";
    size_t distance;
    void *region_start, *region_end;
    
    if (is_left_overflow) {
        distance = (char*)rec->user_addr - (char*)fault_addr;
        region_start = rec->user_addr;
        region_end = (char*)rec->user_addr + rec->user_size;
    } else {
        distance = (char*)fault_addr - ((char*)rec->user_addr + rec->user_size);
        region_start = rec->user_addr;
        region_end = (char*)rec->user_addr + rec->user_size;
    }
    
    printf("%p is located %zu bytes to %s of %zu-byte region [%p,%p)\n",
           fault_addr, distance, direction, rec->user_size, region_start, region_end);
}

/**
 * @brief 打印分配位置信息
 */
void print_allocation_location(struct allocation_record *rec) {
    if (rec->alloc_backtrace_size > 0) {
        printf("allocated by thread T0 here:\n");
        
        for (int i = 0; i < rec->alloc_backtrace_size; i++) {
            printf("    #%d %p\n", i, rec->alloc_backtrace[i]);
        }
    }
}

/**
 * @brief 转发给默认处理器
 */
void forward_to_default_handler(int sig, siginfo_t *info) {
    signal(sig, SIG_DFL);
    raise(sig);
}

/**
 * @brief 完整的SIGSEGV处理器
 */
void sigsegv_handler(int sig, siginfo_t *info, void *context) {
    if (sig != SIGSEGV) return;
    
    void *fault_addr = info->si_addr;
    struct allocation_record *rec = find_allocation(fault_addr);
    
    if (!rec) {
        // 不是我们的保护页，转发给默认处理器
        forward_to_default_handler(sig, info);
        return;
    }

    // =================== 1. 错误头部信息 ==================
    printf("=================================================================\n");
    printf("==%d==ERROR: Toy AddressSanitizer: heap-buffer-overflow on address %p\n", getpid(), fault_addr);

    // =================== 2. 访问信息详情 ==================
    const char *access_type = infer_access_type(info->si_code);
    printf("%s of size 1 at %p thread T0\n", access_type, fault_addr);

    // =================== 3. 当前调用栈 ==================
    print_call_stack();

    printf("\n");
    
    // =================== 4. 内存位置关系 ==================
    print_memory_relation(fault_addr, rec);
    
    printf("\n");
    
    // =================== 5. 分配位置跟踪 ==================
    print_allocation_location(rec);

    // =================== 6. 错误摘要 ==================
    printf("SUMMARY: Toy AddressSanitizer: heap-buffer-overflow in main\n");
    
    printf("=================================================================\n");
    exit(1);
}

// setup_signal_handler() 函数保持不变
```

### 阶段4: 编译配置更新

#### 4.1 修改 `CMakeLists.txt`

**动机**: 确保backtrace功能正常工作

**修改内容**:
```cmake
# 添加编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fno-omit-frame-pointer -rdynamic")

# 添加必要的链接库
target_link_libraries(complete_asan_test PRIVATE ${CMAKE_DL_LIBS})
```

## 🎯 实现优先级

### 高优先级（必须完成）
1. **数据结构扩展** - `toy_asan.h` 添加调用栈字段
2. **toy_malloc增强** - 记录分配时调用栈
3. **信号处理器重写** - 完整错误报告实现

### 中优先级（重要优化）
4. **编译配置** - 确保backtrace正常工作
5. **测试验证** - 验证完整功能

### 低优先级（后续增强）
6. **错误处理增强** - 更好的符号解析
7. **性能优化** - 减少不必要的调用

## 📋 验证计划

### 测试场景
```c
void test_complete_asan() {
    char *buf = toy_malloc(100);    // 记录分配栈
    buf[105] = 'x';              // 触发右溢出SIGSEGV
    
    // 期望输出：
    // 1. 错误头部信息
    // 2. 访问类型和地址
    // 3. 当前调用栈（test_complete_asan → main）
    // 4. 内存位置关系（5 bytes to right）
    // 5. 分配位置（toy_malloc → test_complete_asan → main）
    // 6. 错误摘要
}
```

### 成功标准
- ✅ SIGSEGV正确触发和捕获
- ✅ 完整的ASan风格错误报告
- ✅ 两个调用栈都正确显示
- ✅ 内存关系计算准确
- ✅ 程序在错误报告后正常退出

## 💡 关键设计决策

### 1. 保留完整调用栈
- **决策**: 不跳过toy_malloc，显示完整的分配调用链
- **理由**: 提供完整的内存分配上下文，有助于调试

### 2. 两次backtrace调用
- **决策**: toy_malloc时记录分配栈，信号处理器时记录执行栈
- **理由**: 回答两个不同的问题："内存从哪来"和"错误在哪发生"

### 3. 动态访问类型检测
- **决策**: 从si_code推断READ/WRITE，而非硬编码
- **理由**: 更准确的错误报告，适应不同访问模式

### 4. 分层错误报告
- **决策**: 6个清晰的部分，模仿真实ASan格式
- **理由**: 用户熟悉的格式，易于理解和调试

## 🚀 实施步骤

1. **修改toy_asan.h** - 扩展数据结构和函数声明
2. **更新toy_malloc.c** - 添加调用栈记录
3. **重写signal_handler.c** - 完整错误报告实现
4. **更新CMakeLists.txt** - 添加必要编译选项
5. **编译测试** - 验证功能正常
6. **提交代码** - 分阶段提交实现

这个实现计划将提供完整的ASan功能，包括准确的错误检测、详细的调用栈信息和直观的错误报告。
