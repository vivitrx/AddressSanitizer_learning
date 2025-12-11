# 缓冲区溢出学习实验

## 🎯 学习路径指南

按照以下顺序学习，遵循项目的"**理论 → 实验 → 理解**"三步法：

---

## 📚 第一步：理论学习

### 阅读概念文档
```
📖 03_buffer_overflow.md
```

**学习重点**：
1. **缓冲区溢出基础概念**
   - 什么是栈溢出？什么是堆溢出？
   - 内存布局和栈帧结构
   - 越界写入的后果

2. **关键问题理解**
   - 为什么C语言容易出现缓冲区溢出？
   - 缓冲区溢出为什么如此危险？
   - 现代编译器如何防护缓冲区溢出？

3. **与ASan的关系**
   - ASan的检测目标
   - 保护页机制的作用
   - 零开销检测的原理

---

## 🔬 第二步：实验验证

### 构建实验环境
```bash
mkdir build && cd build
cmake ..
make
```

### 实验1：栈溢出基础
**目标**：理解栈溢出的基本原理
```bash
# 正常情况 - 观察正常运行
./01_basic_stack_overflow "short"

# 溢出情况 - 观察越界写入但不立即崩溃
./01_basic_stack_overflow "this_is_way_too_long"
```

**观察要点**：
- buffer地址和输入地址
- 缓冲区大小vs输入长度
- 程序是否正常结束

### 实验2：返回地址覆盖
**目标**：理解栈溢出的危险性
```bash
# 正常调用
./02_return_address_overflow "short"

# 尝试劫持返回地址
./02_return_address_overflow "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
```

**观察要点**：
- local_var的值变化
- 是否调用了never_called_function
- 程序是否正常返回

### 实验3：堆溢出基础
**目标**：理解堆溢出如何影响相邻内存
```bash
# 正常情况
./01_basic_heap_overflow "normal"

# 溢出情况 - 观察相邻堆块被破坏
./01_basic_heap_overflow "this_will_overflow_into_next_block"
```

**观察要点**：
- buffer1和buffer2的地址距离
- buffer2的内容是否被破坏
- free时是否崩溃

### 实验4：堆元数据破坏
**目标**：理解堆管理器的脆弱性
```bash
# 无溢出 - 正常释放
./02_metadata_corruption 16

# 中等溢出 - 破坏元数据，在free时崩溃
./02_metadata_corruption 32

# 严重溢出 - 立即破坏malloc
./02_metadata_corruption 64
```

**观察要点**：
- free()时是否崩溃
- 后续malloc()是否成功
- 崩溃错误信息类型

### 实验5：函数指针劫持
**目标**：理解控制流劫持的严重性
```bash
# 正常调用
./01_function_pointer_hijack "short"

# 尝试劫持函数指针
./01_function_pointer_hijack "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
```

**观察要点**：
- func_ptr的地址和值变化
- 是否调用了malicious_function
- 程序控制流是否被改变

---

## 💡 第三步：深入理解

### 自我检查问题
1. **概念理解**
   - [ ] 能否解释栈溢出和堆溢出的区别？
   > **答案**：栈溢出发生在栈内存，覆盖返回地址、栈帧指针等控制信息；堆溢出发生在堆内存，破坏相邻堆块或堆管理器元数据。栈溢出通常直接劫持控制流，堆溢出通过破坏元数据间接控制。

   - [ ] 理解为什么缓冲区溢出可以劫持程序控制流？
   > **答案**：C/C++中函数返回地址、函数指针、虚函数表等控制信息都存储在内存中。缓冲区溢出可以覆盖这些关键数据，当程序使用被覆盖的地址时就会跳转到攻击者指定的位置。

   - [ ] 知道内存对齐如何影响溢出效果？
   > **答案**：内存对齐会在变量间插入填充字节，改变实际偏移量。例如32位系统上char[32]后的int可能有3字节填充，64位上函数指针通常8字节对齐。攻击时必须精确计算真实偏移。

2. **实验观察**
   - [ ] 观察到了哪些不同类型的崩溃？
   > **答案**：段错误（SIGSEGV）、总线错误（SIGBUS）、非法指令（SIGILL）、double free/corruption错误等。不同溢出目标产生不同崩溃：返回地址覆盖导致段错误，堆元数据破坏导致malloc/free崩溃。

   - [ ] 理解为什么有些溢出不会立即崩溃？
   > **答案**：溢出破坏的内存可能暂时未被使用。堆溢出破坏的元数据只有在下次malloc/free时才检查，栈溢出覆盖的数据可能在函数返回时才使用。

   - [ ] 能解释不同错误信息的原因？
   > **答案**："Segmentation fault"通常是访问无效地址；"double free or corruption"是堆管理器检测到元数据破坏；"stack smashing detected"是栈保护机制发现栈帧被破坏。

3. **ASan联系**
   - [ ] 理解为什么需要AddressSanitizer？
   > **答案**：传统防护有性能开销且不全面。ASan提供零开销（运行时）检测，能发现所有类型内存越界，包括栈、堆、全局变量，且提供详细错误定位。

   - [ ] 知道ASan如何通过保护页检测这些错误？
   > **答案**：ASan在每个内存区域前后分配不可访问的保护页（PROT_NONE）。任何越界访问都会触发SIGSEGV，ASan的信号处理器分析错误并报告精确的越界位置。

   - [ ] 明白影子内存的作用机制？
   > **答案**：影子内存是ASan的核心，将应用程序内存映射到专门的影子区域，每个字节对应1个影子字节。影子字节标记内存状态：0xFF为 poisoned（不可访问），0为可访问，正值表示部分可访问的字节数。

### 进阶调试练习
如果想更深入理解，可以尝试：

#### 使用GDB观察内存布局
```bash
gdb ./01_basic_stack_overflow
(gdb) break vulnerable_function
(gdb) run "overflow_input"
(gdb) x/20x $rsp  # 查看栈内容
(gdb) info frame   # 查看栈帧信息
```

#### 使用AddressSanitizer对比检测
```bash
# 重新编译带ASan的版本
cd build
cmake -DENABLE_STACK_PROTECTION=ON ..
make clean && make

# 对比有无ASan的检测结果
./01_basic_heap_overflow "overflow_input"
```

#### 使用Valgrind检测内存错误
```bash
valgrind --tool=memcheck ./01_basic_heap_overflow "overflow_input"
```

---

## 🔧 故障排除

### 常见问题
1. **程序没有按预期崩溃**
   - 尝试更长的输入字符串
   - 检查编译器优化级别（使用-O0）
   - 确认没有启用栈保护

2. **段错误但没有详细信息**
   - 使用`dmesg`查看内核日志
   - 用GDB调试获取更多上下文
   - 启用核心转储

3. **编译错误**
   - 确认使用GCC或Clang
   - 检查CMake版本（需要3.10+）
   - 清理build目录重新构建

---

## 🎓 学习成果检查

完成所有实验后，你应该能够：

✅ **理论掌握**
- [ ] 清晰解释缓冲区溢出的原理和类型
- [ ] 理解栈帧和堆布局的差异
- [ ] 知道缓冲区溢出的安全危害

✅ **实验验证**
- [ ] 成功触发各种类型的缓冲区溢出
- [ ] 观察到不同的崩溃行为
- [ ] 理解内存布局对溢出的影响

✅ **实践能力**
- [ ] 能够识别潜在的缓冲区溢出代码
- [ ] 理解ASan的检测原理
- [ ] 掌握基本的调试技巧

---

## 🚀 下一步学习

完成缓冲区溢出学习后，按学习路径继续：

1. **概念4：Use-after-free** (需要从主项目目录访问 `doc/concepts/04_use_after_free.md`)
2. **阶段3：ASan核心原理实验**
3. **阶段4：构建玩具ASan**

---

## ⚠️ 安全警告

- **重要**：这些程序故意包含安全漏洞，仅用于教育目的！
- **禁止**：绝对不要在生产环境运行这些技术
- **原则**：理解攻击是为了更好地防御

---

## 📚 参考资料
- [AddressSanitizer官网](https://clang.llvm.org/docs/AddressSanitizer.html)
---

*🎯 记住：每个实验都要经历"理解原理 → 验证现象 → 深入思考"的过程！*
