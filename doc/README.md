# AddressSanitizer Learning - 文档目录结构

## 📚 文档组织概览

本目录包含了AddressSanitizer学习项目的所有文档，按功能和用途进行分类组织。

```
doc/
├── 📄 README.md                    # 文档导航（本文件）
├── 📄 VS_CODE_DEBUG_GUIDE.md       # VS Code调试完整指南
├── 📄 Low_Level_Collaborator.txt    # 低级协作指南
├── 📄 learning_path.md             # 学习路径指导
├── 📁 analysis/                    # 项目分析文档
│   ├── ADDRESS_TRANSFORMATION_ANALYSIS.md    # addr2line地址转换分析
│   └── DOCUMENTATION_CLEANUP_ANALYSIS.md     # 文档清理分析报告
├── 📁 concepts/                    # 概念教育文档
│   ├── 01_virtual_memory.md        # 虚拟内存概念
│   ├── 02_mmap.md                  # mmap系统调用详解
│   └── 03_buffer_overflow.md       # 缓冲区溢出概念
├── 📁 implementation/              # 实现指导文档
│   ├── addr2line_symbol_resolution_workflow.md    # addr2line符号解析流程
│   ├── backtrace_to_error_report_workflow.md     # 错误报告生成流程
│   ├── signal_handler_implementation_guide.md    # 信号处理器实现指南
│   ├── symbol_resolution_fix_star_method.md       # 符号解析修复方法
│   └── toy_ASan_build_guide.md                  # 构建系统指南
├── 📁 tools/                       # 工具和分析代码
│   ├── analyze_maps.c              # 内存映射分析工具
│   ├── debug_addr.c                # 地址调试工具
│   ├── mmap_comparison.c           # mmap对比工具
│   ├── test_addr2line.c            # addr2line测试工具
│   └── test_maps_analysis.c        # 内存映射测试工具
└── 📁 config/                      # 配置文件
    └── CMakeLists.txt              # 主构建配置
```

## 🎯 使用指南

### 📖 学习路径

1. **新手入门**：
   - 阅读 `learning_path.md` 了解整体学习规划
   - 学习 `concepts/` 目录下的基础概念
   - 参考 `implementation/toy_ASan_build_guide.md` 搭建环境

2. **深入实现**：
   - 查看 `implementation/` 目录下的详细实现指南
   - 研究 `analysis/` 目录下的技术分析
   - 使用 `tools/` 目录下的工具进行实验

3. **调试开发**：
   - 参考 `VS_CODE_DEBUG_GUIDE.md` 进行调试
   - 使用 `Low_Level_Collaborator.txt` 指导协作开发
   - 利用 `tools/` 中的工具进行分析

### 🔧 工具使用

- **内存分析工具**：`analyze_maps.c` - 分析进程内存映射
- **地址调试工具**：`debug_addr.c` - 调试地址转换问题
- **符号解析测试**：`test_addr2line.c` - 测试addr2line功能
- **构建系统**：`config/CMakeLists.txt` - 主构建配置

### 📊 项目分析

- **地址转换分析**：`analysis/ADDRESS_TRANSFORMATION_ANALYSIS.md`
- **文档清理报告**：`analysis/DOCUMENTATION_CLEANUP_ANALYSIS.md`

## 🏗️ 文档维护

### 分类原则

- **concepts/**：理论概念和基础知识
- **implementation/**：具体实现和技术细节
- **analysis/**：项目分析和研究报告
- **tools/**：实用工具和分析代码
- **config/**：配置和构建文件

### 更新指南

1. 新增概念文档 → `concepts/`
2. 实现细节文档 → `implementation/`
3. 分析报告 → `analysis/`
4. 实用工具代码 → `tools/`
5. 配置文件 → `config/`

## 📝 文档历史

- **2024-12-15**：完成文档结构重组，清理临时文件
- **2024-12-14**：添加addr2line和backtrace分析文档
- **2024-12-13**：完善调试指南和工具文档
- **2024-12-06**：初始文档结构建立

---

*本文档结构旨在提供清晰的学习路径和完整的技术文档，帮助开发者深入理解AddressSanitizer的实现原理。*
