# VS Code GUI调试指南

## 快速开始

### 1. 打开调试视图
在VS Code中按 `Ctrl+Shift+D` 打开调试视图，或点击左侧活动栏的调试图标。

### 2. 选择调试配置
在调试配置下拉菜单中选择：
- **"Debug signal_handler_test"** - 完整调试ASan错误处理流程
- **"Debug toy_malloc (Direct)"** - 直接调试toy_malloc函数

### 3. 开始调试
按 `F5` 或点击绿色播放按钮开始调试。

## 关键调试点

### 🔍 调试目标：理解addr2line工作流程

#### 1. 设置断点
在以下关键位置设置断点：

**在 `signal_handler.c` 中：**
- `sigsegv_handler` 函数入口 - 捕获SIGSEGV信号
- `resolve_symbol_with_addr2line` 函数 - 观察地址转换过程
- `get_executable_mapping` 函数 - 查看内存映射解析

**在 `toy_malloc.c` 中：**
- `toy_malloc` 函数 - 观察backtrace记录过程
- 第104行：`rec->alloc_backtrace_size = backtrace(...)`

**在测试文件中：**
- `signal_handler_test.c:43` - 触发溢出的位置
- `signal_handler_test.c:23` - 内存分配位置

#### 2. 调试步骤

**步骤1：启动调试**
1. 选择 "Debug signal_handler_test" 配置
2. 按 F5 开始调试
3. 程序会在 `main` 函数开始处暂停

**步骤2：跟踪内存分配**
1. 设置断点在 `toy_malloc` 函数
2. 单步执行到第104行
3. 观察 `backtrace()` 调用和返回的地址

**步骤3：触发溢出**
1. 继续执行到 `signal_handler_test.c:43`
2. 观察溢出发生时的内存访问
3. 程序会触发SIGSEGV并进入信号处理器

**步骤4：调试信号处理器**
1. 在 `sigsegv_handler` 中单步执行
2. 观察错误信息生成过程
3. 跟踪 `resolve_symbol_with_addr2line` 函数

**步骤5：分析地址转换**
在 `resolve_symbol_with_addr2line` 函数中：
- 检查 `load_base` 和 `file_offset` 的值
- 观察地址计算过程：`file_relative_offset = addr - load_base + file_offset`
- 查看 `addr2line` 命令的构造和执行

### 🎯 关键变量观察

#### 在信号处理器中：
```c
void *fault_addr = info->si_addr;  // 故障地址
struct allocation_record *rec;     // 分配记录
```

#### 在地址解析中：
```c
uintptr_t load_base;      // 加载基址
uintptr_t file_offset;     // 文件偏移
uintptr_t file_relative_offset;  // 最终计算结果
```

#### 在backtrace数据中：
```c
rec->alloc_backtrace[i];   // 分配时的调用栈地址
buffer[i];                 // 错误时的调用栈地址
```

### 🔧 调试技巧

#### 1. 监视表达式
在调试视图中添加以下监视表达式：
- `fault_addr` - 查看触发溢出的地址
- `rec->user_addr` - 查看分配的用户地址
- `rec->user_size` - 查看分配大小
- `load_base` - 查看加载基址
- `file_relative_offset` - 查看计算结果

#### 2. 内存检查
使用内存视图查看：
- `fault_addr` 周围的内存
- `rec->user_addr` 的内存布局
- 保护页的设置

#### 3. 调用栈分析
查看调用栈了解：
- 信号触发的完整路径
- 函数调用关系
- 地址转换的调用上下文

### 📊 验证addr2line工作

#### 手动验证步骤：
1. 在调试器中获取 `file_relative_offset` 的值
2. 在终端中运行：
   ```bash
   addr2line -fi -e build/bin/signal_handler_test <file_relative_offset>
   ```
3. 比较调试器中的输出和手动命令的结果

### 🚨 常见问题

#### 1. 断点不生效
- 确保使用Debug构建
- 检查符号文件是否存在
- 验证编译时包含 `-g` 标志

#### 2. 信号处理器不被调用
- 检查 `LD_PRELOAD` 环境变量
- 确认 `libtoy_asan.so` 正确加载
- 验证信号处理器注册成功

#### 3. addr2line返回"??"
