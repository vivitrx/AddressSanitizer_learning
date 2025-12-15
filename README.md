# Toy AddressSanitizer

一个从零开始构建的最小化AddressSanitizer，用于学习和理解ASan的核心工作原理。

## 项目概述

本项目实现了一个简化版的ASan，使用保护页机制来检测堆缓冲区溢出错误。通过这个项目，你将深入理解：

- Linux虚拟内存管理
- mmap/mprotect系统调用
- 信号处理机制
- 内存错误检测原理

## 快速开始

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 构建玩具ASan库
make toy_asan

# 构建所有测试程序
make all_tests
```

### 运行测试

```bash
# 运行所有测试
make run_tests

# 运行特定测试
make run_overflow_test
```

### 使用玩具ASan检测程序

```bash
# 方法1：LD_PRELOAD
LD_LIBRARY_PATH=build/lib LD_PRELOAD=build/lib/libtoy_asan.so your_program

# 方法2：直接链接
gcc -Lbuild/lib -ltoy_asan your_program.c -o your_program
```

## 项目结构

```
AddressSanitizer_learning/
├── README.md                    # 项目说明
├── doc/                         # 完整文档目录
│   ├── README.md                # 文档导航指南
│   ├── VS_CODE_DEBUG_GUIDE.md   # VS Code调试指南
│   ├── learning_path.md         # 学习路径指南
│   ├── concepts/                # 概念学习文档
│   │   ├── 01_virtual_memory.md # 虚拟内存概念
│   │   ├── 02_mmap.md           # mmap系统调用
│   │   └── 03_buffer_overflow.md # 缓冲区溢出概念
│   ├── implementation/          # 实现文档
│   │   ├── toy_ASan_build_guide.md # 构建指南
│   │   ├── addr2line_symbol_resolution_workflow.md # addr2line流程
│   │   ├── backtrace_to_error_report_workflow.md # 错误报告流程
│   │   ├── signal_handler_implementation_guide.md # 信号处理器
│   │   └── symbol_resolution_fix_star_method.md # 符号解析修复
│   ├── analysis/                # 项目分析文档
│   │   ├── ADDRESS_TRANSFORMATION_ANALYSIS.md # 地址转换分析
│   │   └── DOCUMENTATION_CLEANUP_ANALYSIS.md # 清理分析报告
│   ├── tools/                   # 工具和分析代码
│   │   ├── analyze_maps.c        # 内存映射分析
│   │   ├── debug_addr.c          # 地址调试工具
│   │   ├── mmap_comparison.c     # mmap对比工具
│   │   ├── test_addr2line.c      # addr2line测试
│   │   └── test_maps_analysis.c  # 内存映射测试
│   └── config/                  # 配置文件
│       └── CMakeLists.txt       # 主构建配置
├── src/                         # 源代码
│   ├── toy_asan/                # 玩具ASan实现
│   │   ├── toy_malloc.c         # 内存分配器
│   │   ├── signal_handler.c     # 信号处理器
│   │   ├── metadata.c           # 元数据管理
│   │   ├── toy_asan.h           # 公共头文件
│   │   └── init.c              # 初始化函数
│   ├── tests/                   # 测试程序
│   │   ├── overflow_test.c      # 缓冲区溢出测试
│   │   ├── use_after_free_test.c # Use-after-free测试
│   │   └── normal_test.c        # 正常程序测试
│   └── experiments/             # 学习实验
│       ├── memory_layout.c      # 内存布局实验
│       ├── mmap_protection.c    # mmap保护实验
│       └── signal_test.c        # 信号处理实验
└── build/                       # 构建输出目录
```

## 核心原理

### 内存布局

我们的玩具ASan使用三页内存布局：

```
+----------------------+ 基地址
|     保护页           | PROT_NONE  ← 左边红区
+---------------------- 基地址 + page_size
|     用户数据          | PROT_READ|PROT_WRITE ← 用户看到的空间
+---------------------- 基地址 + 2*page_size
|     保护页           | PROT_NONE  ← 右边红区
+----------------------+
```

### 检测机制

1. **正常访问**：用户程序正常访问中间的用户数据页，没有任何开销
2. **越界访问**：当程序访问保护页时，CPU硬件自动触发SIGSEGV信号
3. **错误捕获**：我们的信号处理器捕获SIGSEGV，判断是否是我们的保护页，并报告错误

### 优势与限制

**优势**：
- ✅ 零运行时开销（纯硬件检测）
- ✅ 实现简单，容易理解
- ✅ 可以检测页面边界的溢出

**限制**：
- ❌ 只能检测跨越页面边界的越界访问
- ❌ 内存开销大（每分配N字节实际使用3×页面大小）
- ❌ 不能检测同一页面内的越界访问

## 学习路径

### 第一阶段：概念学习
1. [虚拟内存基础](doc/concepts/01_virtual_memory.md)
2. [mmap系统调用](doc/concepts/02_mmap.md)
3. [信号处理机制](doc/concepts/03_signals.md)

### 第二阶段：实验验证
1. [内存布局实验](src/experiments/memory_layout.c)
2. [mmap保护实验](src/experiments/mmap_protection.c)
3. [信号处理实验](src/experiments/signal_test.c)

### 第三阶段：实现玩具ASan
1. [构建指南](doc/implementation/toy_ASan_build_guide.md)
2. 基础内存分配器实现
3. 信号处理器集成
4. 元数据管理完善

## 示例程序

### 检测缓冲区溢出

```c
#include <stdlib.h>
#include <string.h>

int main() {
    char *buf = malloc(10);
    strcpy(buf, "This string is way too long for 10 bytes");
    buf[15] = 'x';  // 这会触发ASan检测
    free(buf);
    return 0;
}
```

**预期输出**：
```
Toy ASan initialized!
*** ASan detected buffer overflow! ***
Illegal access at address 0x7f8b12345000
Buffer starts at 0x7f8b12346000
Buffer size: 10 bytes
Access is 4096 bytes left of buffer
Program terminated with signal SIGSEGV
```

## 扩展方向

### 短期扩展
- [ ] Use-after-free检测
- [ ] 更详细的错误报告
- [ ] 线程安全性支持

### 长期扩展
- [ ] 采样机制减少内存开销
- [ ] 影子内存实现更细粒度检测
- [ ] 智能错误分析

## 与真实ASan的对比

| 特性 | 玩具ASan | 真实ASan |
|------|----------|----------|
| 检测精度 | 页面级别 | 字节级别 |
| 运行时开销 | 零开销 | 小开销 |
| 内存开销 | 3×页面大小 | 1/8内存大小 |
| 实现复杂度 | 简单 | 复杂 |
| 检测能力 | 页面边界溢出 | 多种内存错误 |

## 贡献指南

欢迎贡献代码和改进建议！请遵循以下步骤：

1. Fork本项目
2. 创建特性分支
3. 提交你的改动
4. 创建Pull Request

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 致谢

本项目的设计参考了：
- LLVM AddressSanitizer的原始设计
- GWP-ASan的采样保护机制
- 《深入理解计算机系统》(CSAPP)的内存管理章节

---

*开始你的ASan学习之旅吧！*
