/**
 * @file 02_signal_handler_debug.c
 * @brief 增强版信号处理器实验 - 带详细调试信息
 *
 * 专门用于调试信号处理器问题
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 增强版sigaction()处理器
void enhanced_advanced_handler(int sig) {
    printf("🎉 成功！增强版处理器收到信号: %d (%s)\n", sig, strsignal(sig));
    printf("   处理器地址: %p\n", enhanced_advanced_handler);
    printf("   进程ID: %d\n", getpid());
    
    if (sig == SIGINT) {
        printf("   用户按下Ctrl+C - 处理器正确触发！\n");
    }
    
    printf("   sigaction()自动保持处理器\n");
    fflush(stdout); // 确保立即输出
}

// 测试sigaction()的详细版本
void test_sigaction_enhanced(void) {
    printf("\n=== 增强版sigaction()测试 ===\n");
    printf("目标：验证sigaction()处理器是否能正确触发\n\n");
    
    struct sigaction sa;
    struct sigaction old_sa;
    
    // 清零结构体
    memset(&sa, 0, sizeof(sa));
    memset(&old_sa, 0, sizeof(old_sa));
    
    // 设置处理器
    sa.sa_handler = enhanced_advanced_handler;
    sigemptyset(&sa.sa_mask); // 不阻塞其他信号
    sa.sa_flags = 0;          // 默认行为
    
    printf("📝 配置信息：\n");
    printf("   处理器函数地址: %p\n", enhanced_advanced_handler);
    printf("   信号掩码: 空（不阻塞其他信号）\n");
    printf("   标志位: 0（默认行为）\n\n");
    
    // 注册信号处理器 - 添加详细错误检查
    printf("🔧 注册SIGINT处理器...\n");
    int result = sigaction(SIGINT, &sa, &old_sa);
    
    if (result == -1) {
        printf("❌ sigaction()失败！\n");
        printf("   错误代码: %d (%s)\n", errno, strerror(errno));
        printf("   这就是处理器不触发的原因！\n");
        return;
    } else {
        printf("✅ sigaction()注册成功！\n");
        printf("   返回值: %d\n", result);
    }
    
    // 验证注册是否成功
    printf("\n🔍 验证注册状态：\n");
    struct sigaction verify_sa;
    if (sigaction(SIGINT, NULL, &verify_sa) == 0) {
        printf("   当前SIGINT处理器地址: %p\n", verify_sa.sa_handler);
        printf("   期望的处理器地址: %p\n", enhanced_advanced_handler);
        if (verify_sa.sa_handler == enhanced_advanced_handler) {
            printf("   ✅ 处理器地址匹配！\n");
        } else {
            printf("   ❌ 处理器地址不匹配！\n");
        }
    }
    
    printf("\n🎯 测试说明：\n");
    printf("   进程ID: %d\n", getpid());
    printf("   即将进入pause()等待信号...\n");
    printf("   请发送SIGINT信号测试处理器\n");
    printf("   方法1: ./build/bin/03_signal_sender %d 2\n", getpid());
    printf("   方法2: 在另一个终端按Ctrl+C\n");
    printf("   方法3: kill -INT %d\n", getpid());
    printf("\n⏸️ 程序已暂停，等待信号中...\n");
    
    // 等待信号
    printf("📞 调用pause()...\n");
    pause();
    
    printf("\n🎊 pause()返回！信号处理完成！\n");
    printf("   这证明信号处理器正确触发了！\n");
    
    // 清理
    printf("\n🔧 恢复原来的处理器...\n");
    if (sigaction(SIGINT, &old_sa, NULL) == -1) {
        printf("❌ 恢复处理器失败: %s\n", strerror(errno));
    } else {
        printf("✅ 处理器恢复成功\n");
    }
}

// 简单的测试版本
void simple_test(void) {
    printf("\n=== 最简单的信号处理器测试 ===\n");
    
    printf("注册最简单的SIGINT处理器...\n");
    
    if (signal(SIGINT, enhanced_advanced_handler) == SIG_ERR) {
        printf("❌ signal()注册失败: %s\n", strerror(errno));
        return;
    }
    
    printf("✅ signal()注册成功！\n");
    printf("进程ID: %d\n", getpid());
    printf("等待SIGINT信号...\n");
    
    pause();
    
    printf("🎉 pause()返回！signal()处理器工作了！\n");
    
    signal(SIGINT, SIG_DFL);
}

int main(int argc, char *argv[]) {
    printf("🧪 信号处理器调试实验\n");
    printf("进程ID: %d\n", getpid());
    printf("========================================\n");
    
    if (argc != 2) {
        printf("用法: %s <测试模式>\n", argv[0]);
        printf("测试模式:\n");
        printf("  1 - 增强版sigaction()测试\n");
        printf("  2 - 最简单的signal()测试\n");
        return 1;
    }
    
    int mode = atoi(argv[1]);
    
    switch (mode) {
    case 1:
        test_sigaction_enhanced();
        break;
    case 2:
        simple_test();
        break;
    default:
        printf("无效的测试模式: %d\n", mode);
        return 1;
    }
    
    printf("\n✅ 调试测试完成！\n");
    return 0;
}
