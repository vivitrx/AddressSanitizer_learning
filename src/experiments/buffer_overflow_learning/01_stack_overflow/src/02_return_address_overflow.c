#include <stdio.h>
#include <string.h>

void never_called_function() {
    printf("!!! 这个函数不应该被调用 !!!\n");
    printf("!!! 栈溢出成功覆盖了返回地址 !!!\n");
}

void vulnerable_function(char *input) {
    // 修复：调整变量声明顺序，让buffer在低地址，local_var在高地址
    // 这样buffer溢出会先覆盖local_var，然后覆盖旧EBP，最后覆盖返回地址
    char buffer[16];     // 在低地址
    int local_var = 0x12345678;  // 在高地址
    
    printf("=== 内存布局分析 ===\n");
    printf("buffer地址:     %p\n", (void*)buffer);
    printf("local_var地址:  %p (值: 0x%x)\n", (void*)&local_var, local_var);
    printf("buffer到local_var的距离: %ld 字节\n", 
           (long)((char*)&local_var - (char*)buffer));
    
    // 计算需要溢出的距离
    long offset_to_local_var = (long)((char*)&local_var - (char*)buffer);
    long offset_to_return = offset_to_local_var + 8 + 4; // local_var + 旧EBP(4) + 返回地址(4)
    
    printf("覆盖local_var需要 %ld 字节溢出\n", offset_to_local_var);
    printf("覆盖返回地址需要 %ld 字节溢出\n", offset_to_return);
    
    // 故意的缓冲区溢出
    strcpy(buffer, input);
    
    printf("溢出后local_var: 0x%x\n", local_var);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        printf("这个实验演示如何通过栈溢出覆盖局部变量和返回地址\n\n");
        
        printf("修复版本 - 调整了变量顺序确保攻击成功:\n\n");
        
        printf("实验步骤:\n");
        printf("1. 正常输入: %s \"short\"\n", argv[0]);
        printf("2. 覆盖local_var: %s \"1234567890123456789012345678\"\n", argv[0]);
        printf("3. 尝试劫持返回地址: %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB\"\n", argv[0]);
        
        printf("\n预期效果:\n");
        printf("- buffer[16] + 4字节填充 + 4字节(local_var) = 24字节覆盖local_var\n");
        printf("- 继续溢出会覆盖旧EBP和返回地址\n");
        return 1;
    }
    
    printf("=== 返回地址覆盖演示 (修复版) ===\n");
    printf("输入长度: %zu 字节\n", strlen(argv[1]));
    printf("缓冲区大小: 16 字节\n");
    
    vulnerable_function(argv[1]);
    
    printf("如果看到这个消息，说明程序正常返回了\n");
    return 0;
}
