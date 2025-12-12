/**
 * @file test_complete_asan.c
 * @brief 测试完整ASan功能 - 简化版本
 */

#include "../toy_asan/toy_asan.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== 测试完整ASan功能 ===\n");
    
    // 初始化ASan
    toy_asan_init();
    
    printf("分配100字节内存...\n");
    char *buf = toy_malloc(100);
    printf("分配内存: %p\n", buf);
    
    // 正常使用
    strcpy(buf, "Hello World");
    printf("正常使用: %s\n", buf);
    
    printf("即将触发右溢出...\n");
    buf[4096] = 'x';  // 直接触发右溢出（需要4096字节偏移）
    
    // 程序不会执行到这里
    printf("错误：溢出未检测到！\n");
    return 0;
}
