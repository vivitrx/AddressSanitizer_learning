# 玩具ASan构建指南

## 项目目标

从零开始构建一个最小化的AddressSanitizer（ASan），能够检测C程序中的堆缓冲区溢出错误，并理解ASan的核心工作原理。

## 项目架构

### 核心组件设计

```
src/toy_asan/
├── toy_malloc.c        # 内存分配器实现
├── signal_handler.c    # 信号处理器
├── metadata.c          # 元数据管理
├── toy_asan.h          # 公共头文件
└── init.c             # 初始化函数
```

### 内存布局策略

#### 我们的玩具ASan布局
```
+----------------------+ 0x... (基地址)
|     保护页           | PROT_NONE  ← 左边红区
+----------------------+ base + page_size
|     用户数据          | PROT_READ|PROT_WRITE ← 用户看到的空间
+----------------------+ base + 2*page_size
|     保护页           | PROT_NONE  ← 右边红区
+----------------------+
```

#### 与真实ASan的对比
- **真实ASan**：使用影子内存，可以检测更细粒度的错误
- **玩具ASan**：使用保护页，只能检测页面边界的越界访问
- **优势**：零运行时开销，实现简单
- **限制**：只能检测跨越页面边界的越界访问

## 实现阶段

### 阶段1：基础内存分配器

#### 1.1 toy_malloc实现
```c
void* toy_malloc(size_t size) {
    // 计算需要的总大小：3 * page_size
    // 分配3页内存：[保护页][用户数据][保护页]
    // 设置中间页为可读写
    // 返回中间页的地址给用户
}
```

#### 1.2 toy_free实现
```c
void toy_free(void *ptr) {
    // 从用户地址反推基地址
    // 检查是否是我们管理的内存
    // 使用munmap释放整个内存块
}
```

#### 1.3 内存布局计算
```c
// 用户地址 -> 基地址
void* user_to_base(void *user_ptr) {
    return (void*)((uintptr_t)user_ptr - page_size);
}

// 基地址 -> 用户地址  
void* base_to_user(void *base_ptr) {
    return (void*)((uintptr_t)base_ptr + page_size);
}
```

### 阶段2：信号处理器

#### 2.1 基础信号处理
```c
void sigsegv_handler(int sig, siginfo_t *info, void *context) {
    void *fault_addr = info->si_addr;
    
    // 检查是否是我们的保护页
    if (is_our_guard_page(fault_addr)) {
        printf("*** ASan detected buffer overflow! ***\n");
        printf("Illegal access at address %p\n", fault_addr);
        exit(1);
    }
    
    // 不是我们的错误，调用默认处理器
    signal(SIGSEGV, SIG_DFL);
    raise(SIGSEGV);
}
```

#### 2.2 保护页检测
```c
bool is_our_guard_page(void *addr) {
    // 遍历所有分配记录
    // 检查地址是否在保护页范围内
    return found_in_metadata(addr);
}
```

### 阶段3：元数据管理

#### 3.1 分配记录结构
```c
struct allocation_record {
    void *base_addr;      // 整个块的基地址
    void *user_addr;      // 用户看到的地址
    size_t user_size;     // 用户请求的大小
    size_t total_size;    // 实际分配大小
    struct allocation_record *next; // 链表指针
};

// 全局分配记录链表
static struct allocation_record *allocations = NULL;
```

#### 3.2 元数据操作
```c
// 添加分配记录
void add_allocation(void *base, void *user, size_t user_size);

// 查找分配记录
struct allocation_record* find_allocation(void *addr);

// 删除分配记录
void remove_allocation(void *user_addr);
```

### 阶段4：集成和接口

#### 4.1 LD_PRELOAD接口
```c
// 在toy_asan.h中定义
void* malloc(size_t size) __attribute__((alias("toy_malloc")));
void free(void *ptr) __attribute__((alias("toy_free")));

// 其他函数的可选实现
void* calloc(size_t nmemb, size_t size);
void* realloc(void *ptr, size_t size);
```

#### 4.2 初始化函数
```c
__attribute__((constructor))
void toy_asan_init() {
    // 注册信号处理器
    struct sigaction sa;
    sa.sa_sigaction = sigsegv_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    
    printf("Toy ASan initialized!\n");
}
```

## 使用方法

### 编译玩具ASan
```bash
# 编译为共享库
gcc -fPIC -shared -o libtoy_asan.so \
    src/toy_asan/*.c -ldl

# 或者使用Makefile
make
```

### 使用玩具ASan检测程序
```bash
# 方法1：LD_PRELOAD
LD_PRELOAD=./libtoy_asan.so ./your_program

# 方法2：链接时替换
gcc -L. -ltoy_asan your_program.c -o your_program
```

### 测试用例
```c
// test_overflow.c
#include <stdlib.h>
#include <string.h>

int main() {
    char *buf = (char*)toy_malloc(10);
    strcpy(buf, "This string is way too long");
    buf[15] = 'x';  // 应该触发ASan
    toy_free(buf);
    return 0;
}
```

## 设计权衡

### 1. 内存开销
- **选择**：保护所有分配
- **开销**：每分配N字节，实际使用3*page_size字节
- **优点**：简单，检测率高
- **缺点**：内存使用量大

### 2. 检测精度
- **能检测**：跨越页面边界的越界访问
- **不能检测**：同一页面内的越界访问
- **原因**：保护机制基于页面权限

### 3. 性能影响
- **正常访问**：零开销（纯硬件检测）
- **异常处理**：信号处理开销
- **元数据管理**：O(n)查找（可优化为哈希表）

## 扩展方向

### 短期扩展
1. **use-after-free检测**：释放后将页面设为PROT_NONE
2. **更详细的错误报告**：显示分配位置、访问类型等
3. **线程安全性**：添加互斥锁保护元数据

### 长期扩展
1. **采样机制**：随机采样减少内存开销
2. **影子内存**：实现更细粒度的检测
3. **智能分析**：区分不同类型的内存错误

## 调试技巧

### 1. 内存布局查看
```bash
# 查看进程内存映射
cat /proc/$(pidof your_program)/maps
```

### 2. 信号处理调试
```c
// 在信号处理器中添加调试信息
printf("Signal %d at address %p\n", sig, fault_addr);
printf("Base address: %p\n", base_addr);
printf("User address: %p\n", user_addr);
```

### 3. 元数据检查
```c
// 添加元数据打印函数
void print_allocations() {
    struct allocation_record *rec = allocations;
    while (rec) {
        printf("Base: %p, User: %p, Size: %zu\n", 
               rec->base_addr, rec->user_addr, rec->user_size);
        rec = rec->next;
    }
}
```

## 预期输出示例

### 正常情况
```
Toy ASan initialized!
Program runs normally...
```

### 检测到溢出
```
*** ASan detected buffer overflow! ***
Illegal access at address 0x7f8b12345000
Buffer starts at 0x7f8b12346000
Buffer size: 10 bytes
Access is 4096 bytes left of buffer
Program terminated with signal SIGSEGV
```

## 成功标准

完成这个项目后，你应该能够：
1. ✅ 理解ASan的核心工作原理
2. ✅ 实现基本的内存错误检测
3. ✅ 掌握mmap/mprotect/信号处理的使用
4. ✅ 理解系统级编程的挑战和技巧
5. ✅ 为学习真实ASan打下坚实基础

---

*准备好开始编码了吗？让我们从toy_malloc开始！*
