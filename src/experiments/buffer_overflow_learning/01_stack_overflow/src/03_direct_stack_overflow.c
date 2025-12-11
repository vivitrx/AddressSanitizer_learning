#include <stdio.h>
#include <string.h>

void target_function() {
    printf("🎯 成功劫持控制流！\n");
    printf("🎯 栈溢出攻击成功！\n");
    printf("🎯 现在程序跳转到了target_function\n");
}

void vulnerable_function(char *input) {
    char small_buffer[8];  // 非常小的缓冲区
    volatile int *ptr;     // 防止编译器优化
    
    printf("=== 直接栈溢出攻击 ===\n");
    printf("small_buffer地址: %p\n", (void*)small_buffer);
    printf("target_function地址: %p\n", (void*)target_function);
    
    // 显示栈的当前状态
    ptr = (int*)(small_buffer + 32);  // 看看栈上的内容
    printf("栈上的返回地址区域: 0x%p\n", (void*)ptr);
    printf("当前返回地址: 0x%x\n", *ptr);
    
    // 危险的缓冲区溢出
    strcpy(small_buffer, input);
    
    // 再次检查栈的状态
    printf("溢出后返回地址: 0x%x\n", *ptr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <攻击字符串>\n", argv[0]);
        printf("\n这是一个直接的栈溢出攻击演示\n\n");
        
        printf("攻击策略:\n");
        printf("1. 使用8字节的小缓冲区\n");
        printf("2. 用长字符串溢出覆盖返回地址\n");
        printf("3. 劫持到target_function\n\n");
        
        printf("尝试不同的攻击:\n");
        printf("1. %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA\"           # 基础溢出\n", argv[0]);
        printf("2. %s \"$(python -c 'print \"A\"*40)\"           # 更长溢出\n", argv[0]);
        printf("3. %s \"$(python -c 'print \"\\x60\\x11\\x40\\x00\" + \"A\"*32)'\"  # 精确地址覆盖\n", argv[0]);
        return 1;
    }
    
    printf("输入: %s\n", argv[1]);
    printf("输入长度: %zu 字节\n\n", strlen(argv[1]));
    
    vulnerable_function(argv[1]);
    
    printf("❌ 如果看到这个消息，攻击失败了\n");
    return 0;
}
