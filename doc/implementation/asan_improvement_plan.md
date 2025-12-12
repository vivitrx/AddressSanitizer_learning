# Toy AddressSanitizer 改进计划

## 📋 当前问题分析

### **问题1：调用栈符号解析缺失**
**现状**：
```
WRITE of size 1 at 0x7f0bc6b21fff thread T0
    #0 0x564773ec517a     ← 纯16进制地址
    #1 0x7f0bc6960090
    #2 0x564773ec43eb
```

**期望**：
```
WRITE of size 1 at 0x7f0bc6b21fff thread T0
    #0 0x564773ec517a in dangerous_function (/path/test+0x517a)  ← 函数名+偏移
    #1 0x564773ec438b in main (/path/test+0x438b)
    #2 0x7f0bc6960090 in __libc_start_main (libc.so.6+0x24090)
```

### **问题2：右溢出检测范围错误**
**现状**：
```
内存布局: [左保护页4096][用户数据页4096][右保护页4096]
用户缓冲区在页开始位置
右溢出需要访问 buf[4096] 才能触发
```

**问题**：
- `malloc(100)` 返回页开始的地址
- 访问 `buf[101]` 仍然在用户数据页内，不会触发SIGSEGV
- 用户需要访问 `buf[4096]` 才能触发右保护页
- 这意味着第105字节到第4095字节的溢出无法被检测

## 🎯 改进方案

### **方案1：符号解析改进**

#### **1.1 实现addr2line集成**
```c
/**
 * @brief 使用addr2line将地址转换为函数名
 * @param addr 要解析的地址
 * @param executable 可执行文件路径
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 */
void resolve_symbol(void *addr, const char *executable, char *output, size_t output_size) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "addr2line -e %s -f -C %p", executable, addr);
    
    FILE *pipe = popen(cmd, "r");
    if (pipe) {
        if (fgets(output, output_size, pipe)) {
            // 移除换行符
            output[strcspn(output, "\n")] = '\0';
        }
        pclose(pipe);
    }
}

/**
 * @brief 增强的调用栈打印
 */
void print_call_stack_enhanced(void) {
    void *buffer[MAX_BACKTRACE_FRAMES];
    int frames = backtrace(buffer, MAX_BACKTRACE_FRAMES);
    
    char executable[256];
    if (readlink("/proc/self/exe", executable, sizeof(executable)) == -1) {
        strcpy(executable, "unknown");
    }
    
    // 跳过信号处理器本身
    for (int i = 1; i < frames; i++) {
        char symbol[256] = "unknown";
        resolve_symbol(buffer[i], executable, symbol, sizeof(symbol));
        
        printf("    #%d %p in %s\n", i-1, buffer[i], symbol);
    }
}
```

#### **1.2 备选方案：使用dladdr**
```c
#include <dlfcn.h>

/**
 * @brief 使用dladdr进行符号解析
 */
void print_call_stack_dladdr(void) {
    void *buffer[MAX_BACKTRACE_FRAMES];
    int frames = backtrace(buffer, MAX_BACKTRACE_FRAMES);
    
    for (int i = 1; i < frames; i++) {
        Dl_info info;
        if (dladdr(buffer[i], &info) && info.dli_sname) {
            printf("    #%d %p in %s (%s+0x%lx)\n", 
                   i-1, buffer[i], info.dli_sname, 
                   info.dli_fname ? info.dli_fname : "unknown",
                   (char*)buffer[i] - (char*)info.dli_saddr);
        } else {
            printf("    #%d %p in unknown\n", i-1, buffer[i]);
        }
    }
}
```

### **方案2：右溢出检测改进**

#### **2.1 当前问题分析**
```
现状布局:
[左保护页4096][用户数据页4096][右保护页4096]
             ^buf指向这里

问题:
- 用户只申请了100字节
- 但实际上有4096字节可用
- buf[101]...buf[4095]的溢出无法检测
```

#### **2.2 改进方案A：子页保护**
```c
/**
 * @brief 在用户数据页内部设置子页保护
 * 
 * 新布局:
 * [左保护页4096][用户数据][右保护子页][填充][右保护页4096]
 *                ^buf      ^保护边界
 *                100字节    1字节保护
 */
void *toy_malloc_improved(size_t size) {
    size_t ps = get_system_page_size();
    
    // 分配4页：左保护 + 用户数据 + 右保护子页 + 右保护页
    size_t total_size = 4 * ps;
    void *base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    // 设置左保护页
    mprotect(base_addr, ps, PROT_NONE);
    
    // 设置右保护页
    void *right_guard = (char*)base_addr + 3 * ps;
    mprotect(right_guard, ps, PROT_NONE);
    
    // 计算用户缓冲区位置（在第二页开始）
    void *user_addr = (char*)base_addr + ps;
    
    // 关键：在用户数据后设置保护子页
    // 计算保护子页的位置：用户数据 + size
    void *sub_guard = (char*)user_addr + size;
    
    // 确保保护子页在正确的页边界
    size_t sub_guard_offset = (char*)sub_guard - (char*)user_addr;
    if (sub_guard_offset < ps) {
        // 保护子页在用户数据页内，使用mprotect设置
        size_t guard_start = (uintptr_t)sub_guard;
        size_t guard_size = ps - ((char*)sub_guard - (char*)user_addr);
        
        // 需要页面对齐，所以要调整
        size_t aligned_start = guard_start & ~(ps - 1);
        size_t aligned_size = guard_size + (guard_start - aligned_start);
        
        mprotect((void*)aligned_start, aligned_size, PROT_NONE);
    }
    
    return user_addr;
}
```

#### **2.3 改进方案B：重新设计内存布局**
```c
/**
 * @brief 更简洁的内存布局设计
 * 
 * 新布局:
 * [左保护页4096][用户数据size][右保护页4096]
 *             ^buf指向这里
 * 
 * 特点:
 * - 用户数据正好是申请的大小
 * - 左右都是完整的保护页
 * - 内存更紧凑，浪费更少
 */
void *toy_malloc_redesign(size_t size) {
    size_t ps = get_system_page_size();
    
    // 计算用户数据需要的页数
    size_t user_pages = (size + ps - 1) / ps;
    
    // 总大小：左保护页 + 用户数据页 + 右保护页
    size_t total_size = (2 + user_pages) * ps;
    void *base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    // 设置左保护页
    mprotect(base_addr, ps, PROT_NONE);
    
    // 设置右保护页
    void *right_guard = (char*)base_addr + (1 + user_pages) * ps;
    mprotect(right_guard, ps, PROT_NONE);
    
    // 用户缓冲区在左保护页后
    void *user_addr = (char*)base_addr + ps;
    
    return user_addr;
}
```

## 🎯 推荐实施方案

### **阶段1：符号解析改进（优先级：高）**
1. **实现dladdr方案**：更简单，不依赖外部工具
2. **添加编译选项**：确保符号信息保留
3. **测试验证**：对比真实ASan的输出格式

### **阶段2：右溢出检测改进（优先级：高）**
1. **采用方案B**：重新设计内存布局
2. **精确计算**：确保用户数据大小=申请大小
3. **边界测试**：验证各种大小的溢出检测

### **阶段3：性能优化（优先级：中）**
1. **内存使用优化**：减少不必要的内存分配
2. **缓存机制**：符号解析结果缓存
3. **错误报告优化**：减少不必要的计算

## 📋 实施步骤详细计划

### **步骤1：符号解析实现**
1. 修改`signal_handler.c`中的`print_call_stack()`函数
2. 添加`dladdr`相关头文件和链接选项
3. 实现符号解析逻辑
4. 测试不同编译选项的影响

### **步骤2：内存布局重设计**
1. 修改`toy_malloc.c`中的内存分配逻辑
2. 更新`metadata.c`中的地址计算函数
3. 调整测试用例中的溢出偏移
4. 全面测试各种大小的分配

### **步骤3：集成测试**
1. 创建综合测试用例
2. 对比真实ASan的输出
3. 性能基准测试
4. 文档更新

## 🔧 技术考虑

### **符号解析限制**
- **dladdr**：只能解析动态链接的符号
- **addr2line**：需要调试信息，但更准确
- **权衡**：先实现dladdr，再考虑addr2line

### **内存布局权衡**
- **当前方案**：简单但有检测盲区
- **方案A**：复杂，但精确
- **方案B**：平衡了简单性和准确性

### **性能影响**
- **符号解析**：只在错误时发生，影响很小
- **内存布局**：可能增加内存使用，但提高准确性
- **权衡**：准确性比微小性能损失更重要

## 📊 成功标准

### **符号解析成功标准**
- [ ] 调用栈显示函数名和偏移
- [ ] 格式接近真实ASan
- [ ] 处理未知符号的情况
- [ ] 不影响检测性能

### **右溢出检测成功标准**
- [ ] `buf[size]` 立即触发SIGSEGV
- [ ] 支持任意大小的分配
- [ ] 内存使用合理
- [ ] 与左溢出检测一致

## 🚀 下一步行动

1. **确认方案选择**：与团队讨论推荐的实施路径
2. **开始实施**：从符号解析开始，逐步改进
3. **持续测试**：每个改进都要验证功能正确性
4. **文档更新**：保持文档与实现同步

这个改进计划将使Toy AddressSanitizer更加专业和实用，提供接近真实ASan的用户体验。
