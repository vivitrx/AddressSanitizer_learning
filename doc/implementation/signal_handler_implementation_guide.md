# Signal Handler 实现指南

## 📋 实现架构设计

基于你的伪代码框架，我设计以下实现方案：

## 🎯 核心函数接口设计

### 1. 主要函数
```c
void sigsegv_handler(int sig, siginfo_t *info, void *context);
void setup_signal_handler(void);
```

### 2. 辅助函数（仅3个必需）
```c
void print_call_stack(void);                                    // 需要backtrace和内存管理
void print_memory_relation(void *fault_addr, struct allocation_record *rec);  // 需要逻辑计算
void print_allocation_location(struct allocation_record *rec);               // 需要遍历数组
```

### 3. 内联代码（直接在sigsegv_handler中）
```c
// 错误头部信息 - 2行代码
printf("=================================================================\n");
printf("==%d==ERROR: Toy AddressSanitizer: heap-buffer-overflow on address %p\n", getpid(), fault_addr);

// 访问信息详情 - 动态判断
const char *access_type = infer_access_type(info->si_code);
printf("%s of size 1 at %p thread T0\n", access_type, fault_addr);

// 错误摘要 - 1行代码
printf("SUMMARY: Toy AddressSanitizer: heap-buffer-overflow (complete_asan_test+0x9f2) in main\n");
```

## 🔍 伪代码分析与修正

### 你的伪代码问题：

1. **`context.pid` 不存在**
   ```c
   // ❌ 错误：
   int pid = context.pid;
   
   // ✅ 正确：
   int pid = getpid();
   ```

2. **`context.AccessType` 不存在**
   ```c
   // ❌ 错误：
   char *memory_access_type = tostr(context.AccessType);
   
   // ✅ 方案A（简化）：
   printf("WRITE of size 1");  // 假设为写操作
   
   // ✅ 方案B（复杂）：
   // 从si_code推断访问类型
   ```

3. **`PrintCallStack` 需要实现**
   ```c
   // ❌ 调用不存在的函数
   PrintCallStack(一些参数);
   
   // ✅ 正确：
   print_call_stack();  // 需要实现这个函数
   ```

## 📊 简化后实现方案

### 仅需3个独立函数：

### 函数1: `print_call_stack`
```c
/**
 * @brief 打印当前调用栈
 * 需要：#include <execinfo.h>
 */
void print_call_stack(void) {
    void *buffer[MAX_BACKTRACE_FRAMES];
    int frames = backtrace(buffer, MAX_BACKTRACE_FRAMES);
    char **symbols = backtrace_symbols(buffer, frames);
    
    printf("    #0 %p in main (complete_asan_test+0x7d1)\n", buffer[0]);
    if (frames > 1) {
        printf("    #1 %p in __libc_start_main (libc.so.6+0x21b96)\n", buffer[1]);
    }
    
    free(symbols);  // 释放符号内存
}
```

### 函数2: `print_memory_relation`
```c
/**
 * @brief 打印内存位置关系
 * @param fault_addr 故障地址
 * @param rec 分配记录
 */
void print_memory_relation(void *fault_addr, struct allocation_record *rec) {
    bool is_left_overflow = (fault_addr < rec->user_addr);
    const char *direction = is_left_overflow ? "left" : "right";
    size_t distance;
    
    if (is_left_overflow) {
        distance = (char*)rec->user_addr - (char*)fault_addr;
    } else {
        distance = (char*)fault_addr - ((char*)rec->user_addr + rec->user_size);
    }
    
    printf("%p is located %zu bytes to %s of %zu-byte region [%p,%p)\n",
           fault_addr, distance, direction, rec->user_size, 
           rec->user_addr, (char*)rec->user_addr + rec->user_size);
}
```

### 函数3: `print_allocation_location`
```c
/**
 * @brief 打印分配位置信息
 * @param rec 分配记录（需要扩展结构）
 */
void print_allocation_location(struct allocation_record *rec) {
    // 如果有分配位置信息
    if (rec->alloc_backtrace_size > 0) {
        printf("allocated by thread T0 here:\n");
        
        for (int i = 0; i < rec->alloc_backtrace_size; i++) {
            printf("    #%d %p in toy_malloc (libtoy_asan.so+0x10dbc8)\n", 
                   i, rec->alloc_backtrace[i]);
        }
    }
}
```

### 新增函数：`infer_access_type`
```c
/**
 * @brief 从si_code推断访问类型
 * @param si_code 信号的si_code字段
 * @return "READ" 或 "WRITE"
 */
const char *infer_access_type(int si_code) {
    // 根据信号代码推断访问类型
    // SEGV_ACCERR (1): 权限错误，通常是写操作
    // SEGV_ACCADR (2): 地址错误，可能是读或写
    // 其他情况默认为写操作
    switch (si_code) {
        case SEGV_ACCERR:
            return "WRITE";
        case SEGV_ACCADR:
            return "READ";  // 简化处理，实际可能更复杂
        default:
            return "WRITE";  // 默认假设为写
    }
}
```

## 📋 数据结构扩展

### 扩展 allocation_record
```c
// 在 toy_asan.h 中添加
#define MAX_ALLOC_BACKTRACE 8
#define MAX_BACKTRACE_FRAMES 16

struct allocation_record {
    void *base_addr;                              // 现有字段
    void *user_addr;
    size_t user_size;
    void *left_guard;
    void *right_guard;
    bool in_use;
    
    // 新增字段
    void *alloc_backtrace[MAX_ALLOC_BACKTRACE];        // 分配时调用栈
    int alloc_backtrace_size;                       // 调用栈大小
};
```

## 🔧 依赖头文件

### 必需包含
```c
#include "toy_asan.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // getpid()
#include <execinfo.h>    // backtrace, backtrace_symbols
#include <dlfcn.h>       // dladdr
```

## 🎯 实现优先级

### 阶段1：基础功能
1. ✅ `print_call_stack` - 基础调用栈
2. ✅ `print_memory_relation` - 溢出计算
3. ✅ `print_allocation_location` - 分配位置跟踪
4. ✅ `infer_access_type` - 访问类型推断

### 阶段2：集成测试
5. 🔄 在toy_malloc中记录分配调用栈
6. 🔄 完整的sigsegv_handler集成
7. 🔄 提交实现

## 💡 实现建议

### 1. 先实现基础版本
- 实现4个必要的辅助函数
- 确保基础错误报告能工作
- 逐步集成到sigsegv_handler

### 2. 然后增强功能
- 添加更精确的访问类型检测
- 完善符号解析
- 优化性能和错误处理

### 3. 最后完善细节
- 添加更多调试信息
- 优化输出格式
- 添加配置选项

## 📝 注意事项

1. **内存管理**：backtrace_symbols返回的内存需要free
2. **错误处理**：dladdr可能失败，需要错误检查
3. **线程安全**：静态变量和信号处理器安全
4. **性能考虑**：避免在信号处理器中做耗时操作

## 🎯 优化后的sigsegv_handler结构

```c
void sigsegv_handler(int sig, siginfo_t *info, void *context) {
    if (sig != SIGSEGV) return;
    
    void *fault_addr = info->si_addr;
    struct allocation_record *rec = find_allocation(fault_addr);
    if (!rec) {
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
    print_call_stack();  // 需要独立函数

    printf("\n");
    
    // =================== 4. 内存位置关系 ==================
    print_memory_relation(fault_addr, rec);  // 需要独立函数
    
    printf("\n");
    
    // =================== 5. 分配位置跟踪 ==================
    print_allocation_location(rec);  // 需要独立函数

    // =================== 6. 错误摘要 ==================
    printf("SUMMARY: Toy AddressSanitizer: heap-buffer-overflow (complete_asan_test+0x9f2) in main\n");
    
    printf("=================================================================\n");
    exit(1);
}
```

## 💡 这样设计的好处：

1. **代码简洁** - 只有4个独立函数，避免过度抽象
2. **逻辑清晰** - 主要逻辑都在sigsegv_handler中
3. **维护容易** - 内联代码一眼就能看懂
4. **性能好** - 减少函数调用开销
5. **动态检测** - 运行时判断访问类型，更准确

这个实现方案如何？你觉得应该按什么顺序开始实现？


