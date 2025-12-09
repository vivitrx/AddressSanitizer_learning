# 🚀 GDB常用命令速查

## 📋 布局和显示控制

### **退出布局模式**：
```gdb
(gdb) layout asm           # 进入汇编布局
(gdb) layout src           # 返回源码布局
(gdb) layout split         # 分割布局
(gdb) layout regs          # 寄存器布局
(gdb) layout next          # 下一个布局
(gdb) layout prev          # 上一个布局

# 退出所有布局，返回命令行模式
(gdb) tui disable         # 完全禁用TUI模式
# 或者
(gdb) q                   # 退出GDB
```

### **更安全的布局控制**：
```gdb
# 使用Ctrl+X+A快速切换TUI模式
# 在TUI模式下按Ctrl+X+A返回命令行

# 或者使用具体的layout命令
(gdb) layout src           # 源码布局
(gdb) layout regs          # 寄存器布局
(gdb) layout split         # 源码+寄存器分割布局
(gdb) layout asm           # 汇编布局
```

## 🎯 信号调试专用命令

### **基础信号调试**：
```gdb
# 启动GDB
gdb ./02_signal_handler_observable

# 设置断点
(gdb) break observable_handler
(gdb) run 1

# 配置信号处理
(gdb) handle SIGINT nostop noprint pass

# 发送信号测试
(gdb) signal 2
```

### **高级信号调试**：
```gdb
# 查看所有信号信息
(gdb) info signals

# 配置多个信号
(gdb) handle SIGTERM nostop noprint pass
(gdb) handle SIGUSR1 nostop noprint pass

# 条件断点
(gdb) break observable_handler if sig == 2
```

## 🔧 常用调试命令

### **程序控制**：
```gdb
(gdb) run 1                # 运行程序
(gdb) continue             # 继续执行
(gdb) next                # 下一行（不进入函数）
(gdb) step                # 单步执行（进入函数）
(gdb) finish              # 执行完当前函数
(gdb) quit                # 退出GDB
```

### **断点管理**：
```gdb
(gdb) break observable_handler     # 设置断点
(gdb) break main                  # 在main设置断点
(gdb) delete 1                    # 删除断点1
(gdb) disable 1                   # 禁用断点1
(gdb) enable 1                    # 启用断点1
(gdb) info breakpoints            # 查看所有断点
```

### **变量检查**：
```gdb
(gdb) print sig                   # 查看变量值
(gdb) print handler_called       # 查看全局变量
(gdb) info locals                # 查看局部变量
(gdb) info variables             # 查看所有变量
(gdb) watch handler_called        # 设置观察点
```

### **调用栈和源码**：
```gdb
(gdb) bt                         # 查看调用栈
(gdb) frame 0                    # 选择栈帧0
(gdb) list observable_handler     # 显示函数源码
(gdb) list 20,40                # 显示20-40行
(gdb) up                         # 上移栈帧
(gdb) down                       # 下移栈帧
```

## 🚨 TUI模式快速退出

### **遇到问题的快速解决方案**：

#### **方法1：Ctrl+X+A**
```gdb
# 在TUI模式下直接按Ctrl+X+A
# 这是切换TUI模式的最快方法
```

#### **方法2：tui disable命令**
```gdb
(gdb) tui disable
# 完全关闭TUI模式，返回纯命令行
```

#### **方法3：quit重新启动**
```gdb
(gdb) quit
$ gdb ./02_signal_handler_observable
# 退出GDB重新启动，避免TUI模式
```

#### **方法4：使用layout src**
```gdb
(gdb) layout src
# 如果在asm布局中，切换回src布局
```

## 💡 调试最佳实践

### **避免TUI模式问题的建议**：
```gdb
# 1. 启动时禁用TUI
$ gdb --tui ./program     # 启用TUI（如果需要）
$ gdb ./program          # 默认不用TUI（推荐）

# 2. 使用纯命令行模式
(gdb) set pagination off  # 关闭分页
(gdb) set print pretty on # 美化输出
```

### **信号调试推荐配置**：
```gdb
# 在 ~/.gdbinit 中添加这些配置
set pagination off
set print pretty on
handle SIGINT nostop noprint pass
handle SIGTERM nostop noprint pass
```

## 🎯 退出布局的具体步骤

### **如果你在 `layout asm` 中**：
```gdb
# 步骤1：尝试切换布局
(gdb) layout src

# 步骤2：如果不行，禁用TUI
(gdb) tui disable

# 步骤3：快捷键方法
# 按Ctrl+X+A

# 步骤4：最后手段
(gdb) quit
```

### **验证退出成功**：
```gdb
# 应该看到纯命令行提示符，没有分割窗口
(gdb) 
```

## 🚀 完整的信号调试示例

### **从启动到调试的完整流程**：
```bash
# 1. 启动GDB
gdb ./02_signal_handler_observable

# 2. 基础配置
(gdb) break observable_handler
(gdb) handle SIGINT nostop noprint pass
(gdb) run 1

# 3. 程序运行后发送信号（新终端）
./build/bin/03_signal_sender <PID> 2

# 4. 观察调试
(gdb) print sig
(gdb) bt
(gdb) info locals

# 5. 如果意外进入TUI模式
(gdb) tui disable  # 退出TUI
# 或按Ctrl+X+A

# 6. 继续调试
(gdb) continue
```

---

**现在你知道如何处理所有GDB布局问题了！** 🎯

**关键命令**：
- `tui disable` - 退出TUI模式
- `Ctrl+X+A` - 快速切换TUI
- `layout src` - 切换到源码布局

**继续你的信号调试之旅吧！** 🚀
