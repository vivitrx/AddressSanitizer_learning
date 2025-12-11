/**
 * @file 03_signal_sender.c
 * @brief 信号发送工具 - 用于向目标进程发送信号
 * 
 * 这个小程序专门用于向其他进程发送信号，避免Ctrl+C的调试器冲突问题
 * 
 * 使用方法：
 * ./03_signal_sender <进程ID> <信号编号>
 * 
 * @author Signal Learning Project
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void print_usage(const char *program_name) {
    printf("用法: %s <进程ID> <信号编号>\n", program_name);
    printf("\n常见信号编号:\n");
    printf("  2  - SIGINT (中断信号，相当于Ctrl+C)\n");
    printf("  15 - SIGTERM (终止信号)\n");
    printf("  10 - SIGUSR1 (用户自定义信号1)\n");
    printf("  12 - SIGUSR2 (用户自定义信号2)\n");
    printf("\n示例:\n");
    printf("  %s 1234 2    # 向进程1234发送SIGINT\n", program_name);
    printf("  %s 1234 10   # 向进程1234发送SIGUSR1\n", program_name);
    printf("\n获取进程ID的方法:\n");
    printf("  1. 运行目标程序时会显示进程ID\n");
    printf("  2. 使用 ps aux | grep <程序名> 查找\n");
    printf("  3. 使用 pgrep <程序名> 查找\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    pid_t target_pid = atoi(argv[1]);
    int signal_num = atoi(argv[2]);

    if (target_pid <= 0) {
        printf("错误：无效的进程ID %d\n", target_pid);
        return 1;
    }

    if (signal_num <= 0) {
        printf("错误：无效的信号编号 %d\n", signal_num);
        return 1;
    }

    printf("🎯 准备向进程 %d 发送信号 %d (%s)\n", 
           target_pid, signal_num, strsignal(signal_num));
    
    // 检查进程是否存在
    if (kill(target_pid, 0) == -1) {
        perror("检查进程失败");
        printf("进程 %d 可能不存在或权限不足\n", target_pid);
        return 1;
    }

    printf("⏳ 按Enter发送信号，或Ctrl+C取消...\n");
    getchar();

    printf("📤 发送信号 %d 到进程 %d\n", signal_num, target_pid);
    
    if (kill(target_pid, signal_num) == -1) {
        perror("发送信号失败");
        return 1;
    }

    printf("✅ 信号发送成功！\n");
    printf("💡 提示：如果目标进程有信号处理器，它会捕获并处理这个信号\n");

    return 0;
}
