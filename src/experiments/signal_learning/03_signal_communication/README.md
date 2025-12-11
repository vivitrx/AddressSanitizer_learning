# 🎯 概念3：从信号中恢复 - 信号信息提取器

## 📋 项目概述

这是信号学习系列的第三个实验，专注于**从信号中提取详细的错误信息**，为构建AddressSanitizer（ASan）奠定基础。

### **核心目标**：
- 🎯 **掌握信号信息提取** - 从siginfo_t结构获取详细错误信息
- 🔧 **理解上下文分析** - 学习CPU状态保存和调用栈重建
- 🚀 **ASan前置技能** - 为内存错误检测做好准备
- 💡 **深化调试技能** - 系统级问题诊断能力

## 📁 项目结构

```
03_signal_communication/
├── docs/                           # 📚 文档目录
│   ├── 03_signal_recovery_from_signals.md    # 理论学习文档
│   ├── 04_debugging_guide.md               # 调试指南
│   └── 05_learning_guide.md               # 完整学习指南
├── src/                           # 💻 源代码目录
│   └── 03_signal_info_extractor.c           # 主要实验程序
└── README.md                     # 📖 项目说明（本文件）
```

## 🚀 快速开始

### **1. 编译实验**
```bash
cd /home/vvtrx/AddressSanitizer_learning/src/experiments/signal_learning
mkdir -p build && cd build
cmake .. && make 03_signal_info_extractor
```

### **2. 运行实验**
```bash
# 后台运行
./build/bin/03_signal_info_extractor &

# 获取进程ID
ps aux | grep 03_signal_info_extractor

# 发送信号测试
kill -SEGV <PID>
```

### **3. 观察输出**
```
🔍 === CRASH ANALYSIS ===
Signal: 11 (Segmentation fault)
Address: 0x3e80000cda6
Code: 0
PID: 52646
Instruction: 0x7f1299e481b4
=========================

🚀 ASan应用示例:
   检查是否在保护区域: is_redzone(0x3e80000cda6)
   检查是否在堆区域: is_heap_address(0x7f1299e481b4)
   生成错误报告: generate_asan_report(0x7ffdbd9d70e0, 0x0)
=========================
```

## 📚 学习路径

### **第一步：理论学习**
📖 **文档**：`docs/03_signal_recovery_from_signals.md`
- siginfo_t结构详解
- 三参数信号处理器机制
- ucontext_t上下文分析
- ASan应用逻辑

### **第二步：实验实践**
💻 **代码**：`src/03_signal_info_extractor.c`
- 注册多种信号处理器
- 提取详细的信号信息
- 生成ASan风格的错误报告
- 信号安全的信息输出

### **第三步：调试深入**
🔧 **指南**：`docs/04_debugging_guide.md`
- GDB原生调试技巧
- VS Code GUI调试配置
- 关键观察点分析
- 深度调试技能训练

### **第四步：完整学习**
📋 **指南**：`docs/05_learning_guide.md`
- 完整学习顺序
- 实践操作详解
- 学习验证方法
- 成功标准检查

## 🎯 核心概念

### **1. siginfo_t结构**
```c
typedef struct {
    int    si_signo;     // 信号编号
    int    si_errno;     // 错误码
    int    si_code;      // 信号代码
    pid_t  si_pid;      // 发送进程ID
    void   *si_addr;     // 错误地址
    // ... 更多字段
} siginfo_t;
```

### **2. 三参数信号处理器**
```c
void handler(int sig, siginfo_t *info, void *context);
```

### **3. ucontext_t上下文**
```c
// CPU寄存器状态
gregs[REG_RIP]  // 指令指针
gregs[REG_RSP]  // 栈指针
gregs[REG_RBP]  // 基址指针
```

## 🔧 调试方法

### **GDB调试（推荐）**
```bash
gdb ./build/bin/03_signal_info_extractor
(gdb) break advanced_siginfo_handler
(gdb) run
(gdb) signal 11
```

### **VS Code调试**
```json
{
    "name": "🎯 信号信息提取调试",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/bin/03_signal_info_extractor"
}
```

### **关键调试命令**
```gdb
(gdb) print *info
(gdb) print info->si_addr
(gdb) print ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_RIP]
(gdb) x/32x info->si_addr
(gdb) bt
```

## 🚀 ASan应用

### **错误检测逻辑**
```c
void asan_error_handler(int sig, siginfo_t *info, void *context) {
    if (sig == SIGSEGV && info->si_addr) {
        void *addr = info->si_addr;
        
        // 检查是否在保护页
        if (is_redzone(addr)) {
            printf("Heap buffer overflow detected at %p\n", addr);
        }
    }
}
```

### **调用栈重建**
```c
void extract_call_stack(ucontext_t *uc) {
    uintptr_t *bp = (uintptr_t *)uc->uc_mcontext.gregs[REG_RBP];
    
    printf("Call stack:\n");
    for (int i = 0; bp && i < 16; i++) {
        uintptr_t ret = bp[1];
        printf("  #%d: %p\n", i, (void *)ret);
        bp = (uintptr_t *)bp[0];
    }
}
```

## 🎉 学习成果

### **完成概念3后，你将掌握**：

#### **✅ 核心技能**：
- **信号信息提取** - 从siginfo_t获取详细错误信息
- **上下文分析** - 理解CPU状态和调用栈
- **调试技术** - GDB和VS Code高级调试
- **ASan基础** - 内存错误检测的核心原理

#### **✅ 实践能力**：
- **错误诊断** - 根据信号信息判断问题原因
- **内存分析** - 检查内存状态和访问模式
- **调用栈重建** - 从上下文提取执行链路
- **错误报告** - 生成有用的调试信息

#### **✅ 系统理解**：
- **内核-用户交互** - 信号传递机制
- **异常处理** - 从硬件异常到软件信号
- **内存保护** - 页面权限和访问控制
- **调试工具** - 系统级问题诊断

## 📊 支持的信号

| 信号 | 编号 | 描述 | 应用场景 |
|------|------|------|----------|
| SIGSEGV | 11 | 段错误 | 内存访问错误 |
| SIGBUS | 7 | 总线错误 | 对齐问题 |
| SIGFPE | 8 | 浮点异常 | 除零错误 |
| SIGILL | 4 | 非法指令 | 执行无效指令 |
| SIGTERM | 15 | 终止信号 | 正常终止 |

## 🔍 故障排除

### **常见问题**：

#### **1. 编译错误**
```bash
# 确保包含必要的头文件
#include <signal.h>
#include <ucontext.h>
#include <stdint.h>
#include <errno.h>
```

#### **2. 信号未到达**
```bash
# 检查进程状态
ps aux | grep 03_signal_info_extractor

# 确认进程ID正确
kill -SEGV $(pgrep 03_signal_info_extractor)
```

#### **3. 断点未命中**
```gdb
# 检查断点状态
(gdb) info breakpoints

# 确认处理器地址
(gdb) print advanced_siginfo_handler
```

#### **4. 信息不完整**
```c
// 添加错误检查
if (!info) {
    write(STDOUT_FILENO, "Error: siginfo_t is NULL\n", 26);
    return;
}
```

## 🚀 下一步学习

### **阶段2：理解内存错误类型**
1. **概念1：缓冲区溢出** - 堆溢出、栈溢出原理
2. **概念2：Use-after-free** - 释放后使用的危险

### **阶段3：ASan核心原理**
1. **实验1：手动创建保护页** - 理解保护页工作原理
2. **实验2：理解红区概念** - ASan的内存布局

### **阶段4：构建玩具ASan**
1. **第一步：最简单的内存分配器** - 基础内存管理
2. **第二步：添加信号处理** - 集成错误检测
3. **第三步：集成测试** - 完整的ASan实现

## 📞 学习支持

### **文档顺序**：
1. `docs/03_signal_recovery_from_signals.md` - 理论学习
2. `docs/04_debugging_guide.md` - 调试技能
3. `docs/05_learning_guide.md` - 完整指南

### **遇到问题时**：
1. **查看调试指南** - 深入分析问题
2. **使用GDB调试** - 检查程序状态
3. **对比预期输出** - 验证实验结果
4. **参考理论文档** - 理解核心概念

### **成功标准**：
- ✅ **理论理解** - 能解释信号机制
- ✅ **实验成功** - 能提取信号信息
- ✅ **调试熟练** - 能使用GDB分析
- ✅ **应用理解** - 知道ASan原理

## 🎯 总结

**概念3：从信号中恢复** 是从基础信号机制向ASan实现的重要过渡。通过这个实验，你将：

1. **深入理解信号机制** - 从内核到用户空间的完整链路
2. **掌握信息提取技术** - 获取详细的错误上下文
3. **培养系统级调试能力** - 使用GDB进行深度分析
4. **建立ASan基础** - 为内存错误检测做好准备

**准备好进入内存错误类型的学习，向构建完整的ASan迈进一步！** 🚀
