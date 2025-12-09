# 信号机制学习实验

## 📚 实验概述

本实验系列专门用于学习Linux信号机制，为理解AddressSanitizer的核心原理奠定基础。通过渐进式的实验和调试器观察，你将深入理解"软件中断"的概念。

## 🎯 学习目标

### 概念1：什么是信号
- ✅ 理解Linux信号机制
- ✅ 信号就像"软件中断"
- ✅ SIGSEGV是什么？（段错误信号）
- ✅ 信号和异常的关系

### 概念2：信号处理器（待完成）
- 如何注册自定义信号处理函数
- signal() vs sigaction()的区别

### 概念3：从信号中恢复（待完成）
- 学会从信号中获取调试信息
- 如何判断错误发生的地址

## 📁 项目结构

```
src/experiments/signal_learning/
├── 01_signal_basics.c          # 实验1：观察不同类型信号
├── CMakeLists.txt              # CMake构建配置
├── DEBUG_GUIDE.md              # 📖 详细调试指南
├── README.md                   # 本文件
└── .vscode/
    ├── launch.json             # VS Code调试配置
    └── tasks.json              # VS Code任务配置
build/                          # 构建输出目录
└── bin/
    └── 01_signal_basics        # 编译后的可执行文件
```

## 🚀 快速开始

### 1. 环境准备
```bash
# 安装必要工具
sudo apt install gdb cmake build-essential

# 确保VS Code已安装C/C++扩展
```

### 2. 编译项目
```bash
cd src/experiments/signal_learning
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target 01_signal_basics
```

### 3. 开始调试
- 打开VS Code
- 按 `Ctrl+Shift+D` 打开调试面板
- 选择调试配置开始实验

详细步骤请参考：[DEBUG_GUIDE.md](DEBUG_GUIDE.md)

## 📋 实验列表

### ✅ 实验1：信号基础观察
**文件**: `01_signal_basics.c`  
**目标**: 观察不同类型错误产生的信号

**测试案例**:
1. 空指针解引用 → SIGSEGV (11)
2. 栈溢出 → SIGSEGV (11)
3. 除零错误 → SIGFPE (8)
4. 非法内存访问 → SIGSEGV (11)
5. 非法指令 → SIGILL (4)
6. 总线错误 → SIGBUS (7)
7. 程序中止 → SIGABRT (6)

### 🚧 实验2：基础信号处理器
**目标**: 学习signal()函数的使用和局限性

### 🚧 实验3：高级信号处理器
**目标**: 掌握sigaction()的高级功能

### 🚧 实验4：获取调试信息
**目标**: 从siginfo_t结构提取详细错误信息

## 🔧 使用方法

### 命令行运行
```bash
# 查看可用测试
./build/bin/01_signal_basics

# 运行特定测试
./build/bin/01_signal_basics 1  # 空指针解引用
./build/bin/01_signal_basics 3  # 除零错误
```

### VS Code调试
1. 按 `F5` 开始调试
2. 在信号处理器中设置断点
3. 观察变量和调用堆栈

### VS Code任务
按 `Ctrl+Shift+P` → "Tasks: Run Task" → 选择任务

## 🧠 核心概念理解

### 信号是什么？
信号是Linux系统中的一种异步通知机制：
- **类比**: 就像硬件中断打断CPU执行，信号打断进程执行
- **特点**: 异步、不可预测、由内核或进程发送
- **作用**: 通知进程发生了某个事件

### 常见信号类型
| 信号 | 编号 | 名称 | 触发原因 |
|------|------|------|----------|
| SIGSEGV | 11 | Segmentation fault | 非法内存访问 |
| SIGFPE | 8 | Floating point exception | 数学错误 |
| SIGILL | 4 | Illegal instruction | 非法指令 |
| SIGABRT | 6 | Aborted | 程序主动中止 |
| SIGBUS | 7 | Bus error | 总线错误 |

### 信号处理流程
```
1. 程序正常执行
   ↓
2. 触发异常（如空指针解引用）
   ↓
3. 内核检测到异常
   ↓
4. 内核发送相应信号给进程
   ↓
5. 进程执行信号处理函数
   ↓
6. 程序继续或终止
```

## 🎯 与ASan的关系

学习信号机制对理解ASan至关重要：

1. **ASan的核心机制**: ASan通过设置保护页来检测内存越界
2. **信号的作用**: 当程序访问保护页时，触发SIGSEGV信号
3. **错误检测**: ASan的信号处理器分析信号，判断是否是我们的保护页
4. **友好报告**: 提供详细的错误信息而不是简单的段错误

## 📊 学习进度

- [x] 实验1：信号基础观察 ✅
- [ ] 实验2：基础信号处理器 🚧
- [ ] 实验3：高级信号处理器 🚧  
- [ ] 实验4：获取调试信息 🚧

## 🔗 相关资源

- [Linux信号手册页](https://man7.org/linux/man-pages/man7/signal.7.html)
- [GDB调试指南](https://sourceware.org/gdb/documentation/)
- [VS Code调试文档](https://code.visualstudio.com/docs/editor/debugging)

## 🤝 贡献

如果你发现问题或有改进建议，请：
1. 检查现有的实验代码
2. 参考学习路径文档
3. 提出改进建议

---

**开始你的信号学习之旅吧！** 🚀

记住：理解信号机制是掌握ASan的关键一步！
