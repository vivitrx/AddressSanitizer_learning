/**
 * @file 02_signal_handler_observable.c
 * @brief 可观察的信号处理器实验 - 专为VS Code GUI调试器优化
 *
 * 本版本专门用于在VS Code GUI调试器中观察信号处理过程
 * 解决调试器拦截信号的问题
 *
 * @author Signal Learning Project
 * @version 2.0 - GUI Debug Optimized
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 全局变量用于观察信号处理状态
volatile int handler_called = 0;
volatile int last_signal = 0;
volatile sig_atomic_t signal_received = 0;

// 可观察的信号处理器 - 专为调试设计
void observable_handler(int sig) {
    // 1. 立即可观察的标记
    handler_called = 1;
    last_signal = sig;
    signal_received = 1;
    
    // 2. 使用write确保输出（信号安全且立即可见）
    write(STDOUT_FILENO, "🎯 SIGNAL_HANDLER_CALLED\n", 26);
    
    // 3. 设置调试断点 - 在这里可以完美观察
    // 👆 在这一行设置断点！信号会在这里停止！
    
    // 4. 详细的调试信息
    char debug_msg[100];
    int len = snprintf(debug_msg, sizeof(debug_msg), 
        "✅ 信号处理器被调用: %d (%s)\n", sig, strsignal(sig));
    write(STDOUT_FILENO, debug_msg, len);
    
    // 5. 观察处理器状态
    len = snprintf(debug_msg, sizeof(debug_msg), 
        "   handler_called = %d, last_signal = %d\n", handler_called, last_signal);
    write(STDOUT_FILENO, debug_msg, len);
    
    // 6. 可以在这里设置更多断点观察变量
    // volatile int debug_point = 1; // 👆 调试断点观察点
    
    // 7. 刷新输出确保可见
    fsync(STDOUT_FILENO);
}

// 测试可观察的信号处理
void test_observable_signal(void) {
    printf("\n=== 🎯 可观察信号处理器测试 ===\n");
    printf("专为VS Code GUI调试器优化\n");
    printf("目标：在调试器中观察信号处理过程\n\n");
    
    printf("📊 调试信息：\n");
    printf("   进程ID: %d\n", getpid());
    printf("   handler_called地址: %p\n", &handler_called);
    printf("   observable_handler地址: %p\n", observable_handler);
    printf("   全局变量地址: %p\n", &last_signal);
    
    // 重置状态
    handler_called = 0;
    last_signal = 0;
    signal_received = 0;
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = observable_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // 重启被中断的系统调用
    
    printf("\n🔧 注册SIGINT处理器...\n");
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        printf("❌ sigaction失败: %s\n", strerror(errno));
        return;
    }
    printf("✅ 处理器注册成功！\n");
    
    // 验证注册
    struct sigaction verify_sa;
    if (sigaction(SIGINT, NULL, &verify_sa) == 0) {
        printf("🔍 验证结果：\n");
        printf("   当前处理器: %p\n", verify_sa.sa_handler);
        printf("   期望处理器: %p\n", observable_handler);
        printf("   匹配状态: %s\n", 
            (verify_sa.sa_handler == observable_handler) ? "✅ 匹配" : "❌ 不匹配");
    }
    
    printf("\n🎯 调试准备就绪！\n");
    printf("操作步骤：\n");
    printf("1. 在observable_handler函数中设置断点\n");
    printf("2. 按F5继续执行程序\n");
    printf("3. 在新终端发送信号: ./build/bin/03_signal_sender %d 2\n", getpid());
    printf("4. 观察断点命中和变量状态\n");
    printf("\n⏳ 程序等待信号中...\n");
    
    // 使用sleep而不是pause，确保可调试
    printf("📞 进入sleep(30)，等待信号...\n");
    sleep(30);
    
    // 检查结果
    if (handler_called) {
        printf("\n🎉 信号处理成功！\n");
        printf("   handler_called: %d\n", handler_called);
        printf("   last_signal: %d (%s)\n", last_signal, strsignal(last_signal));
    } else {
        printf("\n⏰ 等待超时，未收到信号\n");
    }
    
    // 恢复默认处理
    signal(SIGINT, SIG_DFL);
}

// 简单的可观察测试
void simple_observable_test(void) {
    printf("\n=== 🎯 简单可观察测试 ===\n");
    
    // 重置状态
    handler_called = 0;
    last_signal = 0;
    
    printf("注册简单可观察处理器...\n");
    if (signal(SIGINT, observable_handler) == SIG_ERR) {
        printf("❌ signal()失败: %s\n", strerror(errno));
        return;
    }
    printf("✅ signal()注册成功！\n");
    
    printf("进程ID: %d\n", getpid());
    printf("等待SIGINT信号...\n");
    
    sleep(20);
    
    if (handler_called) {
        printf("🎉 简单测试成功！收到信号: %d\n", last_signal);
    }
    
    signal(SIGINT, SIG_DFL);
}

// 多信号可观察测试
void multi_signal_observable_test(void) {
    printf("\n=== 🎯 多信号可观察测试 ===\n");
    
    // 注册多个信号
    signal(SIGINT, observable_handler);
    signal(SIGTERM, observable_handler);
    signal(SIGUSR1, observable_handler);
    
    printf("已注册多个信号处理器：\n");
    printf("   SIGINT (2)  - Ctrl+C\n");
    printf("   SIGTERM (15) - 终止信号\n");
    printf("   SIGUSR1 (10) - 用户信号\n");
    printf("进程ID: %d\n", getpid());
    
    printf("\n测试命令：\n");
    printf("  kill -INT %d\n", getpid());
    printf("  kill -TERM %d\n", getpid());
    printf("  kill -USR1 %d\n", getpid());
    
    printf("\n⏳ 等待信号（30秒）...\n");
    sleep(30);
    
    if (handler_called) {
        printf("🎉 多信号测试成功！最后收到: %d\n", last_signal);
    }
    
    // 恢复默认处理
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
}

int main(int argc, char *argv[]) {
    printf("🎯 可观察信号处理器实验 - VS Code GUI优化版\n");
    printf("进程ID: %d\n", getpid());
    printf("========================================\n");
    
    if (argc != 2) {
        printf("用法: %s <测试模式>\n", argv[0]);
        printf("测试模式:\n");
        printf("  1 - 可观察sigaction()测试\n");
        printf("  2 - 简单可观察测试\n");
        printf("  3 - 多信号可观察测试\n");
        printf("\n💡 专为VS Code GUI调试器设计！\n");
        printf("   设置断点在observable_handler中观察信号处理过程\n");
        return 1;
    }
    
    int mode = atoi(argv[1]);
    
    switch (mode) {
    case 1:
        test_observable_signal();
        break;
    case 2:
        simple_observable_test();
        break;
    case 3:
        multi_signal_observable_test();
        break;
    default:
        printf("无效的测试模式: %d\n", mode);
        return 1;
    }
    
    printf("\n✅ 可观察测试完成！\n");
    printf("🎯 现在你可以在VS Code GUI中完美观察信号处理了！\n");
    return 0;
}
