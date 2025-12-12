/**
 * @file signal_handler_test.c
 * @brief 信号处理器基础测试
 * 
 * 测试setup_signal_handler()函数是否正确安装SIGSEGV处理器，
 * 并验证处理器能否被正确调用。
 */

#include "../toy_asan/toy_asan.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("=== Signal Handler Test ===\n");
    
    // 测试1：验证信号处理器安装
    printf("1. Installing signal handler...\n");
    setup_signal_handler();
    
    // 测试2：正常分配
    printf("2. Allocating memory...\n");
    char *buf = toy_malloc(100);
    if (!buf) {
        printf("toy_malloc failed\n");
        return 1;
    }
    printf("   Allocated at %p\n", buf);
    
    // 测试3：正常访问（不应该触发信号）
    printf("3. Testing normal access...\n");
    for (int i = 0; i < 10; i++) {
        buf[i] = 'A' + i;
    }
    printf("   Normal access OK\n");
    
    // 测试4：左溢出（应该触发SIGSEGV）
    printf("4. Testing left overflow...\n");
    printf("   About to write to buf[-1]...\n");
    fflush(stdout);
    
    // 这里应该触发SIGSEGV
    buf[-1] = 'X';
    
    // 如果没触发信号，说明有问题
    printf("   ERROR: Left overflow did not trigger SIGSEGV!\n");
    
    toy_free(buf);
    return 0;
}
