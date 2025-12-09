/**
 * @file 01_signal_basics.c
 * @brief 信号基础概念实验 - 观察不同类型错误产生的信号
 * 
 * 本实验演示各种常见错误会触发什么信号，帮助理解：
 * - 信号就像"软件中断"
 * - SIGSEGV是什么？（段错误信号）
 * - 信号和异常的关系
 * 
 * 编译运行：
 * gcc -o 01_signal_basics 01_signal_basics.c && ./01_signal_basics
 * 
 * 注意：每个测试都会导致程序终止，这是正常现象！
 * 
 * @author Signal Learning Project
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// 简单的信号处理器，用于观察信号
void signal_observer(int sig) {
    printf("🚨 捕获到信号: %d (%s)\n", sig, strsignal(sig));
    printf("进程ID: %d\n", getpid());
    printf("这是信号处理器，程序即将终止...\n");
}

// 设置信号观察器
void setup_observer(void) {
    // 捕获各种信号以便观察
    signal(SIGSEGV, signal_observer);  // 段错误
    signal(SIGFPE, signal_observer);   // 浮点异常
    signal(SIGILL, signal_observer);   // 非法指令
    signal(SIGBUS, signal_observer);   // 总线错误
    signal(SIGABRT, signal_observer);  // 中止
}

void test_null_pointer_dereference(void) {
    printf("\n=== 测试1: 空指针解引用 ===\n");
    printf("尝试解引用空指针...\n");
    
    int *null_ptr = NULL;
    printf("空指针地址: %p\n", (void*)null_ptr);
    
    // 这会触发SIGSEGV
    *null_ptr = 42;
}

// 递归函数调用导致栈溢出
static void recursive_function(void) {
    char large_buffer[8192];  // 每次分配8KB栈空间
    printf("递归深度增加，栈地址: %p\n", large_buffer);
    recursive_function();  // 无限递归
}

void test_stack_overflow(void) {
    printf("\n=== 测试2: 栈溢出 ===\n");
    printf("尝试导致栈溢出...\n");
    
    recursive_function();
}

void test_division_by_zero(void) {
    printf("\n=== 测试3: 除零错误 ===\n");
    printf("尝试除以零...\n");
    
    int a = 10;
    int b = 0;
    
    printf("%d / %d = ?", a, b);
    int result = a / b;  // 这会触发SIGFPE
    printf("结果: %d\n", result);
}

void test_invalid_memory_access(void) {
    printf("\n=== 测试4: 非法内存访问 ===\n");
    printf("尝试访问非法内存地址...\n");
    
    // 尝试访问一个很可能是无效的地址
    char *evil_ptr = (char*)0x12345678;
    printf("尝试访问地址: %p\n", evil_ptr);
    
    *evil_ptr = 'X';  // 这会触发SIGSEGV
}

// 包含非法指令的函数
// 在x86-64上，0x0F 0x0B是UD2指令（未定义指令）
static void illegal_code(void) {
    __asm__ volatile("ud2");  // 触发SIGILL
}

void test_illegal_instruction(void) {
    printf("\n=== 测试5: 非法指令 ===\n");
    printf("尝试执行非法指令...\n");
    
    illegal_code();
}

void test_bus_error(void) {
    printf("\n=== 测试6: 总线错误 ===\n");
    printf("尝试导致总线错误...\n");
    
    // 尝试对齐错误访问（在某些架构上会触发SIGBUS）
    // 在x86上可能不会触发，但我们试试看
    char buffer[16] __attribute__((aligned(16)));
    
    printf("缓冲区地址: %p (对齐到16字节)\n", buffer);
    
    // 尝试未对齐的访问
    char *unaligned = buffer + 1;
    printf("尝试未对齐访问地址: %p\n", unaligned);
    
    // 强制转换为int指针并访问
    int *bad_ptr = (int*)unaligned;
    *bad_ptr = 0x12345678;
}

void test_abort(void) {
    printf("\n=== 测试7: 程序中止 ===\n");
    printf("调用abort()函数...\n");
    
    abort();  // 这会触发SIGABRT
}

int main(int argc, char *argv[]) {
    printf("🔍 信号基础实验 - 观察不同类型的信号\n");
    printf("进程ID: %d\n", getpid());
    printf("========================================\n");
    
    // 设置信号观察器
    setup_observer();
    
    if (argc != 2) {
        printf("用法: %s <测试编号>\n", argv[0]);
        printf("可用的测试:\n");
        printf("  1 - 空指针解引用 (SIGSEGV)\n");
        printf("  2 - 栈溢出 (SIGSEGV)\n");
        printf("  3 - 除零错误 (SIGFPE)\n");
        printf("  4 - 非法内存访问 (SIGSEGV)\n");
        printf("  5 - 非法指令 (SIGILL)\n");
        printf("  6 - 总线错误 (SIGBUS)\n");
        printf("  7 - 程序中止 (SIGABRT)\n");
        printf("\n注意：每个测试都会导致程序终止！\n");
        return 1;
    }
    
    int test_num = atoi(argv[1]);
    
    switch (test_num) {
        case 1:
            test_null_pointer_dereference();
            break;
        case 2:
            test_stack_overflow();
            break;
        case 3:
            test_division_by_zero();
            break;
        case 4:
            test_invalid_memory_access();
            break;
        case 5:
            test_illegal_instruction();
            break;
        case 6:
            test_bus_error();
            break;
        case 7:
            test_abort();
            break;
        default:
            printf("无效的测试编号: %d\n", test_num);
            return 1;
    }
    
    printf("测试完成（这行不应该被执行）\n");
    return 0;
}
