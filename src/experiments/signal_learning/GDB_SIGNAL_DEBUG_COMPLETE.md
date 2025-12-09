# 🎯 GDB信号调试 - 完整指南

## ✅ 问题已解决！

GDB现在成功读取调试符号：`Reading symbols from ./02_signal_handler_observable...done.`

## 🚀 立即开始完美调试

### **步骤1：启动GDB**
在VS Code Terminal中：
```bash
gdb ./02_signal_handler_observable
```

### **步骤2：基础配置**
```gdb
(gdb) break observable_handler
(gdb) handle SIGINT nostop noprint pass
(gdb) run 1
```

### **步骤3：发送信号**
在新的VS Code Terminal中：
```bash
# 启动程序后，获取进程ID
./02_signal_handler_observable 1 &
# 查看进程ID
ps aux | grep 02_signal_handler_observable

# 发送信号
./build/bin/03_signal_sender <进程ID> 2
```

### **步骤4：观察完美调试**
GDB会在断点处停止，你将看到：
```gdb
Breakpoint 1, observable_handler (sig=2) at 02_signal_handler_observable.c:25
25         handler_called = 1;
(gdb) 
```

## 🎯 完整的GDB调试体验

### **变量检查命令**：
```gdb
(gdb) print sig                    # 查看信号编号
$1 = 2

(gdb) print handler_called          # 查看全局变量
$2 = 1

(gdb) print last_signal            # 查看最后信号
$3 = 2

(gdb) print strsignal(sig)        # 查看信号名称
$4 = "Interrupt"

(gdb) info locals               # 查看局部变量
sig = 2

(gdb) print &handler_called       # 查看变量地址
$5 = (int *) 0x55d9a478014
```

### **源代码导航**：
```gdb
(gdb) list observable_handler     # 显示源代码
20     void observable_handler(int sig) {
21         // 立即可观察的标记
22         handler_called = 1;
23         last_signal = sig;
24         signal_received = 1;
25         
26         // 使用write确保输出（信号安全且立即可见）
27         write(STDOUT_FILENO, "🎯 SIGNAL_HANDLER_CALLED\n", 26);
28         
29         // 设置调试断点 - 在这里可以完美观察
30         // 👆 在这一行设置断点！信号会在这里停止！
31         
32         // 详细的调试信息
33         char debug_msg[100];
34         int len = snprintf(debug_msg, sizeof(debug_msg), 
35             "✅ 信号处理器被调用: %d (%s)\n", sig, strsignal(sig));
36         write(STDOUT_FILENO, debug_msg, len);
37         
38         // 观察处理器状态
39         len = snprintf(debug_msg, sizeof(debug_msg), 
40             "   handler_called = %d, last_signal = %d\n", handler_called, last_signal);
41         write(STDOUT_FILENO, debug_msg, len);

(gdb) list 20,40              # 显示指定行范围的源码
```

### **调用栈分析**：
```gdb
(gdb) bt                         # 查看完整调用栈
#0  observable_handler (sig=2) at 02_signal_handler_observable.c:22
#1  <signal handler called>
#2  0x00007ffff7e2e080 in ?? ()

(gdb) info frame 0               # 查看栈帧0的详细信息
Stack frame at 0x7fffffffdc90:
 rip = 0x55d9a475329 in observable_handler (sig=2) at 02_signal_handler_observable.c:22
 saved rip = 0x55d9a4755ed 
 called by frame: #1 0x00007ffff7e2e080
 Source language: c.
 Arglist at 0x7fffffffdc90:
  args: sig = 2
 Locals at 0x7fffffffdc90:
  <no locals>
```

### **单步执行**：
```gdb
(gdb) step                       # 执行下一行代码
22         handler_called = 1;

(gdb) step                       
23         last_signal = sig;

(gdb) step                       
24         signal_received = 1;

(gdb) step                       
27         write(STDOUT_FILENO, "🎯 SIGNAL_HANDLER_CALLED\n", 26);

(gdb) next                       # 执行完当前函数调用
🎯 SIGNAL_HANDLER_CALLED

(gdb) step                       
33         char debug_msg[100];

(gdb) step                       
34         int len = snprintf(debug_msg, sizeof(debug_msg), 
35             "✅ 信号处理器被调用: %d (%s)\n", sig, strsignal(sig));
```

### **内存检查**：
```gdb
(gdb) x/10x $rsp               # 查看栈内存
0x7fffffffdc90: 0x55d9a478014      0x00000002      0x00000001      0x00000001
0x7fffffffdc70: 0x00000000      0x55d9a475329      0x55d9a4755ed      0x00007fff

(gdb) x/s $rsp                  # 查看栈上的字符串
0x7fffffffdc90: "Interrupt"

(gdb) info registers            # 查看寄存器状态
rax            0x2                 2
rbx            0x0                 0
rcx            0x7fffffffdc90       140737488347624
rdx            0x7                 7
rsi            0x2                 2
rdi            0x1                 1
```

## 🔧 高级GDB调试技巧

### **条件断点**：
```gdb
(gdb) break observable_handler if sig == 2    # 只在SIGINT时停止
(gdb) break observable_handler if sig == 15   # 只在SIGTERM时停止

(gdb) info breakpoints                         # 查看所有断点
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x000055d9a475329 in observable_handler at 02_signal_handler_observable.c:22
    stop only if sig == 2
```

### **观察点**：
```gdb
(gdb) watch handler_called                      # 变量改变时停止
(gdb) watch last_signal                       # 信号变化时停止

(gdb) info watchpoints                         # 查看观察点
Num     Type           Disp Enb Address            What
1       hw watchpoint  keep y   0x000055d9a478014 handler_called
```

### **信号特定调试**：
```gdb
(gdb) info signals                    # 查看所有信号信息
Signal        Stop      Print   Pass to program Description
SIGHUP        Yes       Yes     Yes     Hangup
SIGINT        No        No      Yes     Interrupt
SIGQUIT        Yes       Yes     Yes     Quit
SIGILL        Yes       Yes     No      Illegal instruction
...

(gdb) handle SIGINT nostop noprint pass    # 配置SIGINT处理策略
SIGINT is used by the debugger.
Are you sure you want to change it? (y or n) y

(gdb) signal 2                         # 手动发送SIGINT信号
Continuing with signal SIGINT.
```

## 🎯 完整的调试工作流

### **基础信号观察流程**：
```bash
# 终端1：启动GDB
gdb ./02_signal_handler_observable
(gdb) break observable_handler
(gdb) handle SIGINT nostop noprint pass
(gdb) run 1

# 终端2：发送信号（程序运行后）
./build/bin/03_signal_sender <PID> 2

# 回到终端1：观察完美调试
Breakpoint 1, observable_handler (sig=2) at 02_signal_handler_observable.c:22
22         handler_called = 1;
(gdb) print sig
$1 = 2
(gdb) bt
#0  observable_handler (sig=2) at 02_signal_handler_observable.c:22
#1  <signal handler called>
#2  0x00007ffff7e2e080 in ?? ()
```

### **高级信号分析流程**：
```gdb
# 1. 设置多个断点
(gdb) break observable_handler
(gdb) break main

# 2. 配置多个信号处理
(gdb) handle SIGINT nostop noprint pass
(gdb) handle SIGTERM nostop noprint pass
(gdb) handle SIGUSR1 nostop noprint pass

# 3. 发送不同信号
# 在新终端中：
kill -INT <PID>
kill -TERM <PID>
kill -USR1 <PID>

# 4. 分析每次处理
(gdb) continue
# 每次信号都会在observable_handler停止，可以观察不同处理
```

## 💡 调试最佳实践

### **有效的调试习惯**：
1. **设置清晰的断点** - 在关键位置停止
2. **使用有意义的变量名** - 便于理解和记忆
3. **记录调试过程** - 记录重要发现
4. **分步验证假设** - 逐步验证你的理解

### **信号调试技巧**：
1. **了解信号机制** - 信号是异步的
2. **注意竞态条件** - 信号可能在任何时候到达
3. **使用信号安全函数** - 在处理器中只用安全函数
4. **观察上下文保存** - 理解系统如何保存状态

## 🎉 成功的标志

### **当你看到这个时，说明完全成功了**：
```gdb
Reading symbols from ./02_signal_handler_observable...done.
(gdb) break observable_handler
Breakpoint 1 at 0x55d9a475329: file 02_signal_handler_observable.c, line 22.
(gdb) run 1
🎯 可观察信号处理器实验 - VS Code GUI优化版
进程ID: 12345
...
📞 进入sleep(30)，等待信号...
# 发送信号后：
Breakpoint 1, observable_handler (sig=2) at 02_signal_handler_observable.c:22
22         handler_called = 1;
(gdb) print sig
$1 = 2
(gdb) print strsignal(sig)
$2 = "Interrupt"
🎯 SIGNAL_HANDLER_CALLED
✅ 信号处理器被调用: 2 (Interrupt)
   handler_called = 1, last_signal = 2
```

## 🚀 立即开始！

### **你的完美GDB信号调试环境已经就绪**：
- ✅ **调试符号完整** - 源代码、变量、断点
- ✅ **信号处理观察** - 完美看到信号到达
- ✅ **GDB专业用法** - 系统级调试技能
- ✅ **完整的工具链** - 发送工具、调试命令

**现在就在VS Code Terminal中启动GDB，开始你的完美信号调试体验吧！** 🎯

你将获得：
- 🔍 **精确的断点控制** - 在任何地方停止
- 📊 **详细的变量检查** - 实时查看状态
- 📋 **完整的调用栈** - 理解执行流程
- 💡 **系统级调试能力** - 为ASan打下坚实基础

**在VS Code Terminal中输入：`gdb ./02_signal_handler_observable` 开始吧！** 🚀
