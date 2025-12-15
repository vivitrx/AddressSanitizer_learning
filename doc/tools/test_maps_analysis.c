#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("PID: %d\n", getpid());
    printf("Press Enter to continue...");
    getchar();
    
    // 简单的函数用于测试地址解析
    printf("Main function address: %p\n", main);
    
    return 0;
}
