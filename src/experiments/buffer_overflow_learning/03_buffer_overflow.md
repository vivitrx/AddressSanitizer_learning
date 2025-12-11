# 概念3：缓冲区溢出

## 理论部分

### 什么是缓冲区溢出？

缓冲区溢出（Buffer Overflow）是指程序向一个缓冲区写入了超过其容量的数据，导致数据覆盖到相邻的内存区域。这是最常见也最危险的内存安全漏洞之一。

### 缓冲区溢出的类型

#### 1. 栈溢出（Stack Buffer Overflow）

**发生位置**：函数栈帧上的局部数组
**栈帧结构**：
```
+------------------+  高地址
|   函数参数        |
+------------------+
|   返回地址        |  ← 溢出可能覆盖这里
+------------------+
|   旧ebp          |  ← 溢出可能覆盖这里  
+------------------+
|   局部变量        |  ← 缓冲区在这里
|   char buf[100]  |
+------------------+  低地址
```

**危险后果**：
- 覆盖返回地址 → 控制程序执行流
- 覆盖其他局部变量 → 数据破坏
- 覆盖保存的寄存器 → 程序崩溃

#### 2. 堆溢出（Heap Buffer Overflow）

**发生位置**：通过malloc/new分配的堆内存
**堆结构**：
```
+------------------+
|   堆块元数据      |  ← 溢出可能破坏这里
+------------------+
|   用户数据        |
|   char *buf      |  ← 缓冲区在这里
+------------------+
|   堆块元数据      |  ← 溢出可能破坏这里
+------------------+
```

**危险后果**：
- 破坏堆管理元数据 → 后续分配失败
- 覆盖相邻堆块 → 数据泄露或破坏
- 堆喷射攻击 → 安全漏洞

### 为什么缓冲区溢出如此危险？

1. **内存布局可预测**：栈和堆的相对位置是固定的
2. **控制流劫持**：可以覆盖函数指针、返回地址等
3. **隐蔽性强**：可能不会立即崩溃，难以发现
4. **利用简单**：只需精心构造的输入数据

### 内存对齐与溢出

```c
// 考虑内存对齐的情况
struct {
    char buffer[8];     // 8字节
    int secret;         // 4字节，但可能对齐到8字节
} data;

// buffer[8]到buffer[15]的写入会覆盖secret
```

## 实验部分

### 实验1：栈溢出基础

让我们创建一个简单的栈溢出例子：

```c
// 文件：src/experiments/buffer_overflow_learning/01_stack_overflow/src/01_basic_stack_overflow.c
#include <stdio.h>
#include <string.h>

void vulnerable_function(char *input) {
    char buffer[16];  // 只有16字节的缓冲区
    printf("buffer地址: %p\n", (void*)buffer);
    printf("input地址:  %p\n", (void*)input);
    
    // 危险：没有长度检查的复制
    strcpy(buffer, input);
    
    printf("复制完成，buffer内容: %s\n", buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        printf("试试不同长度的输入观察效果\n");
        return 1;
    }
    
    printf("=== 栈溢出演示 ===\n");
    vulnerable_function(argv[1]);
    return 0;
}
```

### 实验2：返回地址覆盖

演示更危险的栈溢出：

```c
// 文件：src/experiments/buffer_overflow_learning/01_stack_overflow/src/02_return_address_overflow.c
#include <stdio.h>
#include <string.h>

void never_called_function() {
    printf("!!! 这个函数不应该被调用 !!!\n");
    printf("!!! 栈溢出成功覆盖了返回地址 !!!\n");
}

void vulnerable_function(char *input) {
    char buffer[32];
    int local_var = 0x12345678;
    
    printf("buffer地址:     %p\n", (void*)buffer);
    printf("local_var地址:  %p (值: 0x%x)\n", (void*)&local_var, local_var);
    
    // 故意的缓冲区溢出
    strcpy(buffer, input);
    
    printf("溢出后local_var: 0x%x\n", local_var);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        printf("试试: %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n", argv[0]);
        return 1;
    }
    
    printf("=== 返回地址覆盖演示 ===\n");
    vulnerable_function(argv[1]);
    
    printf("正常从vulnerable_function返回\n");
    return 0;
}
```

### 实验3：堆溢出基础

```c
// 文件：src/experiments/buffer_overflow_learning/02_heap_overflow/src/01_basic_heap_overflow.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        return 1;
    }
    
    printf("=== 堆溢出演示 ===\n");
    
    // 分配两个相邻的堆块
    char *buffer1 = malloc(16);
    char *buffer2 = malloc(16);
    
    printf("buffer1地址: %p\n", (void*)buffer1);
    printf("buffer2地址: %p\n", (void*)buffer2);
    printf("距离: %ld 字节\n", (long)(buffer2 - buffer1));
    
    // 初始化buffer2
    strcpy(buffer2, "SECRET_DATA");
    printf("buffer2初始内容: %s\n", buffer2);
    
    // 向buffer1溢出写入
    printf("向buffer1写入 %zu 字节的数据...\n", strlen(argv[1]));
    strcpy(buffer1, argv[1]);  // 危险：没有长度检查
    
    printf("溢出后buffer2内容: %s\n", buffer2);
    
    free(buffer1);
    free(buffer2);
    return 0;
}
```

### 实验4：元数据破坏

```c
// 文件：src/experiments/buffer_overflow_learning/02_heap_overflow/src/02_metadata_corruption.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入长度>\n", argv[0]);
        printf("试试不同长度观察堆管理器的行为\n");
        return 1;
    }
    
    int overflow_size = atoi(argv[1]);
    printf("=== 堆元数据破坏演示 ===\n");
    printf("溢出大小: %d 字节\n", overflow_size);
    
    // 分配缓冲区
    char *buffer = malloc(16);
    printf("分配的缓冲区地址: %p\n", (void*)buffer);
    
    // 故意溢出写入，破坏堆元数据
    for (int i = 0; i < overflow_size; i++) {
        buffer[i] = 'A';
    }
    printf("溢出写入完成\n");
    
    // 尝试释放，可能会崩溃
    printf("尝试释放缓冲区...\n");
    free(buffer);  // 这里可能会崩溃
    
    // 尝试再次分配，观察行为
    printf("尝试再次分配...\n");
    char *new_buffer = malloc(32);
    if (new_buffer) {
        printf("新分配成功: %p\n", (void*)new_buffer);
        free(new_buffer);
    } else {
        printf("新分配失败\n");
    }
    
    return 0;
}
```

### 实验5：函数指针劫持

```c
// 文件：src/experiments/buffer_overflow_learning/03_consequences/src/01_function_pointer_hijack.c
#include <stdio.h>
#include <string.h>

void legitimate_function() {
    printf("这是合法函数\n");
}

void malicious_function() {
    printf("!!! 恶意函数被调用了 !!!\n");
    printf("!!! 缓冲区溢出成功劫持了函数指针 !!!\n");
}

struct {
    char buffer[32];
    void (*func_ptr)(void);
} data;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        return 1;
    }
    
    printf("=== 函数指针劫持演示 ===\n");
    
    // 初始化
    data.func_ptr = legitimate_function;
    printf("buffer地址: %p\n", (void*)data.buffer);
    printf("func_ptr地址: %p\n", (void*)&data.func_ptr);
    printf("func_ptr值: %p\n", (void*)data.func_ptr);
    
    // 正常调用
    printf("调用函数指针:\n");
    data.func_ptr();
    
    // 缓冲区溢出，覆盖函数指针
    printf("\n执行缓冲区溢出...\n");
    strcpy(data.buffer, argv[1]);
    
    printf("溢出后func_ptr值: %p\n", (void*)data.func_ptr);
    
    // 再次调用，可能会调用恶意函数
    printf("\n再次调用函数指针:\n");
    data.func_ptr();
    
    return 0;
}
```

## 理解部分

### 关键问题解答

**Q: 栈溢出和堆溢出有什么区别？**
A: 
- **位置不同**：栈溢出发生在函数栈帧，堆溢出发生在动态分配的内存
- **检测难度**：栈溢出通常更容易触发崩溃，堆溢出可能更隐蔽
- **利用方式**：栈溢出主要劫持返回地址，堆溢出主要破坏元数据

**Q: 为什么C语言容易出现缓冲区溢出？**
A: 
- **不检查边界**：strcpy、gets等函数不检查长度
- **手动内存管理**：程序员需要自己计算缓冲区大小
- **指针算术**：可以直接操作内存地址

**Q: 现代编译器如何防护缓冲区溢出？**
A:
- **栈保护（Stack Canary）**：在返回地址前插入随机值
- **ASLR**：地址空间布局随机化
- **FORTIFY_SOURCE**：编译时检查某些危险函数
- **AddressSanitizer**：运行时检测越界访问

### 与ASan的关系

理解缓冲区溢出对ASan至关重要：

1. **检测目标**：ASan专门设计来检测各种缓冲区溢出
2. **保护页机制**：通过在缓冲区周围设置不可访问页面捕获溢出
3. **影子内存**：跟踪每字节内存的可访问状态
4. **零开销检测**：在编译时插入检测代码，运行时高效捕获

### 调试技巧

1. **使用GDB观察内存**：
   ```bash
   gdb ./program
   (gdb) break vulnerable_function
   (gdb) run "AAAAAAAAAAAAAAAA"
   (gdb) x/20x $esp  # 查看栈内容
   (gdb) info frame   # 查看栈帧信息
   ```

2. **查看进程内存映射**：
   ```bash
   cat /proc/$(pidof program)/maps
   ```

3. **使用Valgrind检测内存错误**：
   ```bash
   valgrind --tool=memcheck ./program "input"
   ```

## 安全编程建议

1. **使用安全的字符串函数**：
   - `strncpy` 代替 `strcpy`
   - `strncat` 代替 `strcat`
   - `snprintf` 代替 `sprintf`

2. **始终检查边界**：
   ```c
   if (input_length < buffer_size) {
       memcpy(buffer, input, input_length);
   }
   ```

3. **使用现代C++特性**：
   - `std::string` 代替 `char*`
   - `std::vector` 代替原始数组
   - `std::array` 代替C数组

## 下一步

理解了缓冲区溢出的危险后，我们就可以学习：
- **Use-after-free**：另一种常见的内存错误
- **ASan核心原理**：如何通过保护页检测溢出
- **玩具ASan实现**：亲手构建内存错误检测工具

---

*准备好进入下一个概念了吗？让我们学习Use-after-free错误！*
