# 🎯 概念3：从信号中恢复 - 学习指南

## 📋 学习目标

### **核心目标**：
- 🎯 **掌握信号信息提取** - 从信号处理中获取详细的错误信息
- 🔧 **理解siginfo_t结构** - 学习详细的信号上下文
- 🚀 **ASan前置技能** - 为内存错误检测做好准备
- 💡 **系统级编程** - 深入理解信号处理机制

### **关键技能**：
1. **三参数信号处理器** - `(int sig, siginfo_t *info, void *context)`
2. **上下文信息提取** - 获取CPU状态、调用栈
3. **错误地址定位** - 精确获取错误发生位置
4. **安全错误报告** - 信号安全的信息输出机制

## 🔧 核心概念

### **1. siginfo_t结构体详解**

#### **结构体定义**：
```c
#include <signal.h>
#include <bits/siginfo.h>

typedef struct {
    int    si_signo;     // 信号编号
    int    si_errno;     // 错误码
    int    si_code;      // 信号代码
    pid_t  si_pid;      // 发送进程ID
    uid_t  si_uid;      // 发送用户ID
    void   *si_addr;     // 错误地址（内存相关信号）
    int    si_status;    // 退出状态
    union sigval si_value; // 信号值
    // ... 更多字段
} siginfo_t;
```

#### **关键字段解释**：
- **si_signo** - 信号编号（SIGSEGV、SIGBUS等）
- **si_code** - 信号的具体代码（SEGV_MAPERR、SEGV_ACCERR等）
- **si_addr** - 触发信号的内存地址
- **si_pid** - 发送信号的进程ID（如果是kill()发送）

### **2. 三参数信号处理器**

#### **处理器原型**：
```c
void handler(int sig, siginfo_t *info, void *context);
```

#### **参数说明**：
- **sig** - 信号编号
- **info** - 详细的信号信息（siginfo_t结构）
- **context** - CPU上下文（寄存器状态、栈指针等）

#### **注册方式**：
```c
struct sigaction sa;
sa.sa_sigaction = handler;        // 使用sa_sigaction字段
sa.sa_flags = SA_SIGINFO;        // 启用三参数处理器
sigaction(SIGSEGV, &sa, NULL);   // 注册处理器
```

### **3. ucontext_t上下文结构**

#### **x86_64架构关键寄存器**：
```c
#ifdef __x86_64__
typedef struct {
    // 通用寄存器
    greg_t gregs[NGREG];
    
    // 浮点寄存器
    fpregset_t fpregs;
    
    // 其他信息
    unsigned long __reserved1[8];
} mcontext_t;
```

#### **重要寄存器索引**：
```c
#define REG_RIP    14  // 指令指针
#define REG_RSP    19  // 栈指针
#define REG_RBP    10  // 基址指针
#define REG_RAX    0   // 返回值寄存器
```

## 🎯 实际应用场景

### **1. 内存错误检测**
```c
void memory_error_handler(int sig, siginfo_t *info, void *context) {
    if (sig == SIGSEGV) {
        void *fault_addr = info->si_addr;
        
        // 判断错误类型
        switch (info->si_code) {
            case SEGV_MAPERR:
                printf("地址映射错误: %p\n", fault_addr);
                break;
            case SEGV_ACCERR:
                printf("访问权限错误: %p\n", fault_addr);
                break;
        }
        
        // 检查是否在堆区域
        if (is_heap_address(fault_addr)) {
            printf("堆溢出检测！\n");
        }
    }
}
```

### **2. 调用栈重建**
```c
void stack_trace_handler(int sig, siginfo_t *info, void *context) {
    if (context) {
        ucontext_t *uc = (ucontext_t *)context;
        uintptr_t *bp = (uintptr_t *)uc->uc_mcontext.gregs[REG_RBP];
        
        printf("调用栈:\n");
        for (int i = 0; i < 10 && bp; i++) {
            uintptr_t ret = bp[1];  // 返回地址
            printf("  #%d: %p\n", i, (void *)ret);
            bp = (uintptr_t *)bp[0];  // 下一个栈帧
        }
    }
}
```

### **3. ASan错误报告**
```c
void asan_error_handler(int sig, siginfo_t *info, void *context) {
    if (sig == SIGSEGV && info->si_addr) {
        void *addr = info->si_addr;
        
        // 检查是否在保护页
        if (is_redzone(addr)) {
            printf("=================================================================\n");
            printf("ERROR: AddressSanitizer detected buffer overflow\n");
            printf("Fault address: %p\n", addr);
            
            // 获取出错指令
            ucontext_t *uc = (ucontext_t *)context;
            void *pc = (void *)uc->uc_mcontext.gregs[REG_RIP];
            printf("Instruction pointer: %p\n", pc);
            
            // 生成详细报告
            generate_asan_report(addr, pc);
            printf("=================================================================\n");
        }
    }
    
    // 退出程序
    _exit(1);
}
```

## 🚀 信号处理的最佳实践

### **1. 信号安全函数**
在信号处理器中只能使用异步信号安全函数：
```c
// ✅ 安全的函数
write()
_exit()
signal()
sigprocmask()

// ❌ 不安全的函数
printf()  // 使用write()替代
malloc()  // 避免内存分配
strcpy()  // 避免字符串操作
```

### **2. 错误恢复策略**
```c
void recovery_handler(int sig, siginfo_t *info, void *context) {
    // 记录错误信息
    log_error(sig, info, context);
    
    // 尝试恢复
    if (is_recoverable_error(sig, info)) {
        // 修复错误状态
        fix_error_state(info);
        
        // 设置错误标志
        error_occurred = 1;
        
        return;  // 从处理器返回，继续执行
    }
    
    // 不可恢复的错误
    cleanup_and_exit(sig, info);
}
```

### **3. 调试信息提取**
```c
void debug_handler(int sig, siginfo_t *info, void *context) {
    char debug_info[500];
    int len = snprintf(debug_info, sizeof(debug_info),
        "=== SIGNAL DEBUG INFO ===\n"
        "Signal: %d (%s)\n"
        "Address: %p\n"
        "Code: %d (%s)\n"
        "PID: %d\n"
        "UID: %d\n"
        "Instruction: %p\n",
        sig, strsignal(sig),
        info ? info->si_addr : NULL,
        info ? info->si_code : -1,
        info ? info->si_pid : 0,
        info ? info->si_uid : 0,
        context ? get_instruction_pointer(context) : NULL);
    
    write(STDOUT_FILENO, debug_info, len);
}
```

## 🔍 调试技巧

### **1. GDB观察信号处理**
```gdb
# 设置断点
(gdb) break advanced_handler

# 运行程序
(gdb) run

# 手动发送信号
(gdb) signal 11

# 检查siginfo_t
(gdb) print *info
(gdb) print info->si_addr
(gdb) print info->si_code

# 检查上下文
(gdb) print *((ucontext_t *)context)
(gdb) info registers
```

### **2. 内存状态分析**
```gdb
# 查看出错地址周围的内存
(gdb) x/32x info->si_addr

# 查看栈内存
(gdb) x/32x $rsp

# 查看指令
(gdb) x/8i $rip
```

### **3. 调用栈分析**
```gdb
# 完整调用栈
(gdb) bt

# 获取栈帧信息
(gdb) frame 0
(gdb) info frame

# 查看局部变量
(gdb) info locals
```

## 🎯 学习检查点

### **理论掌握检查**：
- [ ] 理解siginfo_t结构的每个字段
- [ ] 掌握三参数处理器的使用
- [ ] 了解ucontext_t上下文结构
- [ ] 知道信号安全函数的限制

### **实践技能检查**：
- [ ] 能够从信号中提取错误地址
- [ ] 能够获取CPU寄存器状态
- [ ] 能够生成信号安全的错误报告
- [ ] 能够在GDB中调试信号处理

### **ASan应用检查**：
- [ ] 理解如何通过信号检测内存错误
- [ ] 知道如何获取错误发生的精确位置
- [ ] 掌握如何生成有用的错误报告
- [ ] 了解错误恢复的基本策略

## 🚀 下一步学习

完成概念3后，你将：
- ✅ **掌握信号信息提取** - 获得详细的错误上下文
- ✅ **具备ASan基础** - 为内存错误检测做好准备
- ✅ **提升调试技能** - 系统级问题诊断能力
- ✅ **深化系统理解** - 内核-用户空间交互机制

**准备好进入阶段2：理解内存错误类型！** 🎯
A1 记录了导致信号的错误内存地址，比如说当我访问一个受保护的内存地址时（比如说0x0000001）就会触发异常信号，而si_addr就会存储0x0000001

A2 为了使用比普通信号处理器更强大的3信号处理器，简单的信号处理器signal()只能知道发生了什么异常，但是我们往往需要知道发生了什么异常，是什么进程引发的异常，以及到底是读取/写入哪个地址导致的异常

A3 首先，ucontext_t是信号触发的瞬间时CPU寄存器状态的快照，也就是说，他保存了所有通用寄存器和标志寄存器和指令指针寄存器（RIP）还有栈信息还有信号屏蔽集合的值

A4 因为printf的内部实现大致为获取锁 -> 解析字符串 -> 将字符串写入到缓冲区 -> 将缓冲区内容写入到标准输出 -> 释放锁，如果主线程执行printf到写入缓冲区的阶段，突然来了一个信号SIGINT，并调用了自定义的信号处理函数，而函数中也使用了printf，就会触发死锁，因为同一个线程试图获取两次锁，第一次是线程的main中的printf获取锁而未释放，第二次是线程的自定义信号处理函数的printf尝试获取锁

A5 仍然是使用ucontext_t，你可以通过ucontext_t的REG_RIP成员（实际上就是指令指针寄存器RIP的快照）获取导致错误的指令的地址。
uc_mcontext.gregs[REG_RIP]
UserContext_MachineContext.GeneralRegisterS[REGister_RegisterInstructionPointer]

Q6 
(gdb) print *info

Q7
(gdb) print *info
然后检查si_code，si_code存储的值为SEGV_MAPERR，那就是地址映射错误，如果是SEGV_ACCERR，那就是权限错误

Q8 假设用户请求了一个4KB大小的页面，那么实际上ASan会使用ASan_malloc分配3个连续页面，然后把中间的页面返回给用户，第一个和第三个页面为保护页，被设置成不可读也不可写（无权限PROT_NONE），如果用户尝试写入一个大于4KB的数据（超过用户合法页面的边界），那么就必然访问到那些无权限页面，此时就会触发段错误，操作系统内核就会发送SIGSEGV信号，然后被ASan的信号处理函数获取到信号，然后信号处理函数就会根据siginfo_t.si_code存储了SEGV_ACCERR知道是用户访问了不该访问的页面，也就是堆溢出

Q9 这个问题有点困难，我回答不了