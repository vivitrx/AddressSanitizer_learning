# 🔍 概念3调试指南 - 信号信息提取

## 🎯 调试目标

### **学习目标**：
- 🔍 **观察信号信息提取** - 实时查看siginfo_t结构
- 📊 **掌握上下文分析** - 理解CPU状态保存
- 🚀 **ASan应用实践** - 为内存检测做准备
- 💡 **深化调试技能** - 系统级问题诊断

## 🔧 实验准备

### **编译实验**：
```bash
cd /home/vvtrx/AddressSanitizer_learning/src/experiments/signal_learning

# 编译概念3实验
gcc -g -o build/bin/03_signal_info_extractor \
    03_signal_communication/src/03_signal_info_extractor.c

# 检查编译结果
ls -la build/bin/03_signal_info_extractor
```

### **基础测试**：
```bash
# 运行程序
./build/bin/03_signal_info_extractor

# 在新终端发送信号
kill -SEGV <PID>
```

## 🎯 调试方法

### **方法1：GDB原生调试（推荐）**

#### **启动GDB**：
```bash
gdb ./build/bin/03_signal_info_extractor
```

#### **设置断点**：
```gdb
(gdb) break advanced_siginfo_handler
Breakpoint 1 at 0x...: file 03_signal_info_extractor.c, line 25.
```

#### **运行和触发**：
```gdb
(gdb) run
🎯 信号信息提取器实验
目标：从信号中获取详细的错误信息
...
⏳ 等待信号...

# 手动发送信号
(gdb) signal 11
Continuing with signal SIGSEGV.
```

#### **观察断点命中**：
```
Breakpoint 1, advanced_siginfo_handler (sig=11, info=0x7fffffffdc90, ucontext=0x7fffffffdc10) 
    at 03_signal_info_extractor.c:25
25         signal_received = 1;
```

### **方法2：VS Code GUI调试**

#### **启动调试**：
1. **打开VS Code**
2. **选择调试配置** - "🎯 信号信息提取调试"
3. **按F5启动**
4. **程序运行等待信号**

#### **发送信号**：
```gdb
# 在Debug Console中输入
-exec signal 11
```

#### **观察GUI响应**：
- ✅ **断点命中** - 在处理器中停止
- ✅ **变量显示** - 实时查看状态变化
- ✅ **调用栈** - GUI显示完整链路
- ✅ **局部变量** - 查看info、ucontext结构

## 🔍 关键调试观察点

### **1. siginfo_t结构观察**

#### **GDB命令**：
```gdb
# 查看完整siginfo_t结构
(gdb) print *info
$1 = {
  si_signo = 11,
  si_errno = 0,
  si_code = 128,
  si_pid = 0,
  si_uid = 0,
  si_addr = 0x0,
  si_status = 0,
  si_band = 0
}

# 查看特定字段
(gdb) print info->si_signo
$2 = 11

(gdb) print info->si_code
$3 = 128

(gdb) print info->si_addr
$4 = (void *) 0x0
```

#### **重要观察点**：
- **si_signo** - 信号编号（11=SIGSEGV）
- **si_code** - 信号代码（128=SI_USER）
- **si_addr** - 错误地址（0x0表示空指针）
- **si_pid** - 发送进程ID

### **2. ucontext_t上下文观察**

#### **寄存器状态**：
```gdb
# 查看完整上下文
(gdb) print *((ucontext_t *)ucontext)

# 查看寄存器数组
(gdb) print ((ucontext_t *)ucontext)->uc_mcontext.gregs

# 查看特定寄存器
(gdb) print ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_RIP]
$5 = 140737488349424
```

#### **栈信息**：
```gdb
# 查看栈指针
(gdb) print ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_RSP]
$6 = 140737488345568

# 查看基址指针
(gdb) print ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_RBP]
$7 = 140737488345616
```

### **3. 全局变量状态观察**

#### **实时变量变化**：
```gdb
# 单步执行
(gdb) step
26         signal_number = sig;

(gdb) print signal_number
$8 = 11

(gdb) step
27         if (info) {

(gdb) print fault_address
$9 = 0x0

(gdb) step
28             fault_address = info->si_addr;

(gdb) print fault_address
$10 = 0x0
```

#### **变量地址验证**：
```gdb
# 查看变量地址
(gdb) print &signal_received
$11 = (volatile sig_atomic_t *) 0x555555558014

(gdb) print &fault_address
$12 = (void **) 0x555555558018

(gdb) print &instruction_pointer
$13 = (volatile long *) 0x555555558020
```

## 🚀 深度调试技巧

### **1. 内存状态分析**

#### **查看出错地址周围**：
```gdb
# 查看出错地址前后的内存
(gdb) x/16x info->si_addr
0x0:     Cannot access memory at address 0x0

# 查看栈内存
(gdb) x/32x $rsp
0x7fffffffdc90: 0x00000000      0x00000000      0x00000000      0x00000000
0x7fffffffdca0: 0x00007fff      0x5edd8340      0x00000000      0x00000000
```

#### **查看指令**：
```gdb
# 查看出错指令
(gdb) x/4i $rip
=> 0x5555555552a2 <main+82>:    mov    DWORD PTR [rip+0x2f8a],eax
   0x5555555552a8 <main+88>:    cmp    eax,0x0
   0x5555555552aa <main+90>:    je     0x5555555552e2 <main+146>
```

### **2. 调用栈重建**

#### **手动栈遍历**：
```gdb
# 获取RBP
(gdb) print $rbp
$14 = 140737488345616

# 遍历栈帧
(gdb) set $i = 0
(gdb) while ($i < 10 && $rbp != 0)
 >print *(void**)$rbp+8
 >set $rbp = *(void**)$rbp
 >set $i = $i + 1
 >end
```

### **3. 信号处理流程分析**

#### **查看调用栈**：
```gdb
# 完整调用栈
(gdb) bt
#0  advanced_siginfo_handler (sig=11, info=0x7fffffffdc90, ucontext=0x7fffffffdc10)
#1  <signal handler called>
#2  0x00007ffff7e9f1b4 in __GI___clock_nanosleep ()
#3  0x0000555555555340 in main (argc=1, argv=0x7fffffffe018)
```

#### **查看栈帧信息**：
```gdb
# 当前栈帧
(gdb) info frame
Stack frame at 0x7fffffffdc90:
 rip = 0x555555555329 in advanced_siginfo_handler; saved rip = 0x555555555540
 called by frame at 0x7fffffffde10
 Arglist at 0x7fffffffdc90:
  sig = 11
  info = 0x7fffffffdc90
  ucontext = 0x7fffffffdc10
 Locals at 0x7fffffffdc90:
  <no locals>
```

## 📊 调试输出分析

### **预期输出示例**：

#### **程序启动输出**：
```
🎯 信号信息提取器实验
目标：从信号中获取详细的错误信息
=====================================
✅ SIGSEGV处理器注册成功
✅ SIGBUS处理器注册成功
✅ SIGFPE处理器注册成功
✅ SIGILL处理器注册成功
✅ SIGTERM处理器注册成功

📊 调试信息:
   进程ID: 12345
   signal_received地址: 0x555555558014
   fault_address地址: 0x555555558018
   instruction_pointer地址: 0x555555558020
   advanced_siginfo_handler地址: 0x555555555329

🎯 测试方法:
   方法1: kill -SEGV 12345
   方法2: kill -BUS 12345
   方法3: kill -FPE 12345
   方法4: kill -ILL 12345
   方法5: kill -TERM 12345
   方法6: GDB调试，然后signal <信号编号>

⏳ 等待信号...
```

#### **信号到达输出**：
```
🔍 === CRASH ANALYSIS ===
Signal: 11 (Segmentation fault)
Address: (nil)
Code: 128
PID: 0
Instruction: 0x555555555540
=========================

📍 分析: 地址映射错误
   地址 (nil) 不在进程地址空间中
   可能原因: 空指针访问、无效地址

📊 栈信息:
   RBP: 0x7fffffffdc50
   RSP: 0x7fffffffdc90
   可在GDB中使用: x/32x 0x7fffffffdc50

🚀 ASan应用示例:
   检查是否在保护区域: is_redzone((nil))
   检查是否在堆区域: is_heap_address((nil))
   生成错误报告: generate_asan_report((nil), 0x555555555540)
=========================
```

## 🎯 ASan应用分析

### **错误检测逻辑**：
```c
// 在ASan中的应用
if (is_redzone(fault_address)) {
    // 检测到堆溢出
    printf("Heap buffer overflow detected at %p\n", fault_address);
    
    // 获取分配信息
    allocation_info *alloc = find_allocation(fault_address);
    if (alloc) {
        printf("Original allocation: %p, size: %zu\n", 
               alloc->ptr, alloc->size);
        printf("Access offset: %ld\n", 
               (char*)fault_address - (char*)alloc->ptr);
    }
}
```

### **调用栈重建**：
```c
// 从上下文重建调用栈
void extract_call_stack(ucontext_t *uc) {
    uintptr_t *bp = (uintptr_t *)uc->uc_mcontext.gregs[REG_RBP];
    int frame_count = 0;
    
    printf("Call stack:\n");
    while (bp && frame_count < 16) {
        uintptr_t ret = bp[1];
        printf("  #%d: %p\n", frame_count++, (void *)ret);
        bp = (uintptr_t *)bp[0];
    }
}
```

## 🔧 常见问题和解决

### **问题1：信号未到达**
```bash
# 检查进程状态
ps aux | grep 03_signal_info_extractor

# 检查信号掩码
cat /proc/<PID>/status | grep Sig
```

**解决方法**：
- 确认进程正在运行
- 检查信号是否被阻塞
- 使用正确的信号编号

### **问题2：断点未命中**
```gdb
# 检查断点状态
(gdb) info breakpoints

# 检查处理器地址
(gdb) print advanced_siginfo_handler
```

**解决方法**：
- 确认处理器正确注册
- 检查SA_SIGINFO标志
- 验证信号发送方式

### **问题3：信息不完整**
```gdb
# 检查siginfo_t是否为NULL
(gdb) print info
$1 = (siginfo_t *) 0x0

# 检查ucontext是否为NULL
(gdb) print ucontext
$2 = (void *) 0x0
```

**解决方法**：
- 某些信号可能不提供完整信息
- 检查信号类型和代码
- 添加额外的错误检查

## 🎉 学习验证

### **完成标准**：
- [ ] **成功提取信号信息** - fault_address、instruction_pointer
- [ ] **理解siginfo_t结构** - 每个字段的意义
- [ ] **掌握GDB调试** - 断点、变量、内存检查
- [ ] **理解ASan应用** - 错误检测、报告生成

### **技能验证**：
- [ ] **能够手动触发信号** - 使用kill或GDB命令
- [ ] **能够分析信号原因** - 根据地址和代码判断
- [ ] **能够提取调用栈** - 从上下文重建栈帧
- [ ] **能够应用ASan逻辑** - 模拟内存错误检测

## 🚀 下一步

完成概念3后，你将：
- ✅ **掌握信号信息提取** - 获得详细的错误上下文
- ✅ **具备ASan基础** - 为内存错误检测做好准备
- ✅ **提升调试技能** - 系统级问题诊断能力
- ✅ **深化系统理解** - 内核-用户空间交互机制

**准备好进入阶段2：理解内存错误类型！** 🎯
