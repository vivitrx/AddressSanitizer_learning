/**
 * @file complete_asan_test.c
 * @brief 完整的ASan功能测试
 * 
 * 测试玩具AddressSanitizer的完整功能：
 * - 左溢出检测
 * - 右溢出检测
 * - 正常分配和释放
 * - 错误处理
 */

#include "../toy_asan/toy_asan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_left_overflow() {
    printf("\n=== 测试1: 左溢出检测 ===\n");
    
    char *buf = toy_malloc(50);
    printf("分配内存: %p (大小: 50)\n", buf);
    
    // 正常写入
    strcpy(buf, "Hello World");
    printf("正常写入: %s\n", buf);
    
    printf("尝试左溢出: buf[-1] = 'X'\n");
    buf[-1] = 'X'; // 应该触发SIGSEGV
    
    printf("错误: 左溢出未触发SIGSEGV!\n");
    toy_free(buf);
}

void test_right_overflow() {
    printf("\n=== 测试2: 右溢出检测 ===\n");
    
    char *buf = toy_malloc(10);
    printf("分配内存: %p (大小: 10)\n", buf);
    
    // 正常写入
    strcpy(buf, "12345");
    printf("正常写入: %s\n", buf);
    
    printf("尝试右溢出: buf[10] = 'X'\n");
    buf[10] = 'X'; // 应该触发SIGSEGV
    
    printf("错误: 右溢出未触发SIGSEGV!\n");
    toy_free(buf);
}

void test_normal_operations() {
    printf("\n=== 测试3: 正常操作 ===\n");
    
    // 测试多次分配和释放
    for (int i = 0; i < 3; i++) {
        size_t size = (i + 1) * 20;
        char *buf = toy_malloc(size);
        printf("分配 %d: %p (大小: %zu)\n", i, buf, size);
        
        // 正常使用
        for (size_t j = 0; j < size - 1; j++) {
            buf[j] = 'A' + (j % 26);
        }
        buf[size - 1] = '\0';
        printf("内容: %s\n", buf);
        
        toy_free(buf);
        printf("释放 %d: 成功\n", i);
    }
}
// // // // // // // // // // // // // // // // // // // // // // // // // // // 

// 函数声明
char * allocate_100byte();
void dangerous_function();

void test_complete_asan_functionality() {
    printf("\n=== 测试4: 完整ASan功能测试 ===\n");
    
    char *buf = allocate_100byte();
    printf("分配内存: %p\n", buf);
    
    // 正常使用
    strcpy(buf, "Hello World");
    printf("正常使用: %s\n", buf);
    
    printf("即将触发右溢出...\n");
    dangerous_function();  // 这里会触发完整的ASan报告
    
    // 程序不会执行到这里
    printf("错误：溢出未检测到！\n");
}

char * allocate_100byte(){
    return toy_malloc(100);
}

void dangerous_function() {    
    char *buf = allocate_100byte();  // 需要先分配内存
    buf[105] = 'x';                 // ← 触发SIGSEGV
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // 
int main() {
    printf("=== 完整ASan功能测试 ===\n");
    
    // 初始化ASan
    toy_asan_init();
    
    // 运行所有测试
    test_normal_operations();
    
    printf("\n注意: 下面的测试会触发SIGSEGV并终止程序\n");
    printf("如果程序正常结束，说明ASan未正确工作\n");
    
    // 这些测试会触发SIGSEGV
    test_left_overflow();
    
    // 如果执行到这里，说明左溢出检测失败
    printf("\n=== 错误: 左溢出检测失败 ===\n");
    
    test_right_overflow();
    
    // 如果执行到这里，说明右溢出检测失败
    printf("\n=== 错误: 右溢出检测失败 ===\n");
    
    // 运行完整ASan功能测试
    test_complete_asan_functionality();
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}
