#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void simple_sigaction_handler(int sig) {
    printf("✅ sigaction处理器成功触发！\n");
    printf("   信号编号: %d\n", sig);
    printf("   信号名称: %s\n", strsignal(sig));
    printf("   处理器地址: %p\n", simple_sigaction_handler);
}

int main() {
    printf("🧪 简单sigaction测试\n");
    printf("进程ID: %d\n", getpid());
    printf("================================\n");
    
    struct sigaction sa;
    sa.sa_handler = simple_sigaction_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction失败");
        return 1;
    }
    
    printf("✅ sigaction注册成功\n");
    printf("⏸️ 等待SIGINT信号 (Ctrl+C 或 kill -INT %d)\n", getpid());
    printf("⏳ 程序暂停中...\n");
    
    pause();
    
    printf("🎉 信号处理完成，程序继续\n");
    
    return 0;
}
