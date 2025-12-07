# 概念2：mmap系统调用

## 理论部分

### 什么是mmap？

`mmap`（memory map）是Linux提供的系统调用，用于将文件或设备映射到进程的虚拟地址空间。它也可以用于分配匿名内存（不与文件关联的内存）。

### mmap的基本语法

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, 
           int fd, off_t offset);
```

**参数解释：**
- `addr`: 建议的映射地址（通常为NULL，让内核选择）
- `length`: 映射区域的字节数
- `prot`: 内存保护权限
- `flags`: 映射类型标志
- `fd`: 文件描述符（匿名映射时设为-1）
