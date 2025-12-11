#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <溢出长度>\n", argv[0]);
        printf("这个实验演示堆溢出如何破坏堆管理器的元数据\n\n");
        
        printf("实验建议:\n");
        printf("1. 无溢出: %s 16\n", argv[0]);
        printf("2. 轻微溢出: %s 20\n", argv[0]);
        printf("3. 中等溢出: %s 32\n", argv[0]);
        printf("4. 严重溢出: %s 64\n", argv[0]);
        
        printf("\n观察要点:\n");
        printf("- free()时是否崩溃\n");
        printf("- 后续malloc()是否成功\n");
        printf("- 程序是否正常退出\n");
        return 1;
    }
    
    int overflow_size = atoi(argv[1]);
    printf("=== 堆元数据破坏演示 ===\n");
    printf("缓冲区大小: 16 字节\n");
    printf("溢出大小: %d 字节\n", overflow_size);
    
    // 分配缓冲区
    char *buffer = malloc(16);
    if (!buffer) {
        printf("内存分配失败\n");
        return 1;
    }
    
    printf("分配的缓冲区地址: %p\n", (void*)buffer);
    
    // 故意溢出写入，破坏堆元数据
    printf("执行溢出写入...\n");
    for (int i = 0; i < overflow_size; i++) {
        buffer[i] = 'A';
    }
    printf("溢出写入完成 (写入了 %d 字节)\n", overflow_size);
    
    // 尝试释放，可能会崩溃
    printf("尝试释放缓冲区...\n");
    free(buffer);  // 这里可能会崩溃
    
    printf("释放成功！\n");
    
    // 尝试再次分配，观察行为
    printf("尝试再次分配...\n");
    char *new_buffer = malloc(32);
    if (new_buffer) {
        printf("新分配成功: %p\n", (void*)new_buffer);
        free(new_buffer);
    } else {
        printf("新分配失败 - 堆可能已损坏\n");
    }
    
    printf("程序正常结束\n");
    return 0;
}
