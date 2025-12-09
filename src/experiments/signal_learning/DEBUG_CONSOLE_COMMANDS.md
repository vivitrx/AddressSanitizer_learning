# 🔧 调试控制台命令速查

## 🎯 信号调试快速设置

如果你发现调试配置没有生效，可以在调试控制台中手动输入以下命令：

### **方法1：在调试控制台中设置命令**

启动调试后，在VS Code调试控制台（Debug Console）中输入：

```gdb
handle SIGINT nostop noprint pass
break advanced_handler
continue
```

### **方法2：手动断点设置**

1. **启动调试**（使用任何配置）
2. **在调试控制台输入**：
```gdb
handle SIGINT nostop noprint pass
```
3. **在代码中手动设置断点**：
   - 在`advanced_handler`函数第一行点击设置断点
   - 按`F5`继续执行

### **方法3：使用调试控制台发送信号**

```gdb
# 手动发送信号给当前进程
signal 2
```

## 🔍 完整调试流程

### **步骤1：启动调试**
1. 按`F5`启动调试
2. 程序在`main()`处停止

### **步骤2：设置信号处理**
在调试控制台输入：
```gdb
handle SIGINT nostop noprint pass
break advanced_handler
```

### **步骤3：运行到等待状态**
```gdb
continue
```

### **步骤4：发送信号**
在新终端：
```bash
./build/bin/03_signal_sender <PID> 2
```

### **步骤5：观察断点命中**
调试器应该在`advanced_handler`处停止

## 🛠️ 调试命令参考

### **信号处理命令**
```gdb
handle SIGINT nostop noprint pass  # 让SIGINT传递给程序
handle SIGINT stop print pass       # 在SIGINT时停止
handle SIGSEGV stop print          # 在SIGSEGV时停止
info signals                        # 查看所有信号处理设置
```

### **断点命令**
```gdb
break advanced_handler              # 在函数设置断点
break 02_signal_handler.c:85        # 在行号设置断点
info breakpoints                    # 查看所有断点
delete breakpoints                  # 删除所有断点
```

### **执行控制**
```gdb
continue                            # 继续执行
next                                # 单步跳过
step                                # 单步进入
finish                              # 跳出当前函数
```

### **信号相关调试**
```gdb
signal 2                            # 手动发送SIGINT
signal SIGUSR1                      # 发送SIGUSR1
info signal                         # 查看最后接收的信号
```

## 🔧 常见问题解决

### **问题1：调试控制台不可见**
**解决**：
1. 按 `Ctrl+Shift+Y` 打开调试控制台
2. 或在菜单中选择 `View > Debug Console`

### **问题2：命令不执行**
**解决**：
1. 确保在调试暂停状态下输入命令
2. 命令前面不要加`(gdb)`前缀
3. 确保命令语法正确

### **问题3：断点不命中**
**解决**：
1. 检查函数名是否正确
2. 确保信号处理设置正确
3. 验证信号是否正确发送

## 💡 最佳实践

### **推荐的调试会话**：
1. **启动调试** → 任意配置
2. **调试控制台** → `handle SIGINT nostop noprint pass`
3. **手动断点** → 点击`advanced_handler`行号
4. **继续执行** → `F5`
5. **发送信号** → 使用信号发送工具
6. **观察断点** → 应该在处理器中停止

### **调试技巧**：
- 使用`continue`而不是`run`来继续执行
- 在信号处理器中观察调用栈
- 监视`sig`变量查看信号编号
- 使用`step`逐行执行处理器代码

---

**🚀 现在你可以完全控制信号调试过程！**

如果调试配置不工作，使用调试控制台命令是100%可靠的方法。
