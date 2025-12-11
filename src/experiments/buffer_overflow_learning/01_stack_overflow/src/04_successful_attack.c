#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void success_function() {
    printf("\n🎉🎉🎉 栈溢出攻击成功！ 🎉🎉🎉\n\n");
    printf("✅ 成功劫持了程序控制流\n");
    printf("✅ 程序跳转到了target_function\n");
    printf("✅ 这证明了缓冲区溢出的危险性\n\n");
    
    printf("攻击详情:\n");
    printf("- 覆盖了栈上的返回地址\n");
    printf("- 函数返回时跳转到了攻击者控制的地址\n");
    printf("- 这就是为什么需要AddressSanitizer！\n");
    
    exit(0);
}

void vulnerable_function(char *input) {
    char buffer[8];  // 小缓冲区，容易溢出
    int dummy = 0xdeadbeef;  // 栈上的其他数据
    
    printf("=== 成功栈溢出攻击演示 ===\n");
    printf("buffer地址:      %p\n", (void*)buffer);
    printf("success_function地址: %p\n", (void*)success_function);
    
    // 显示攻击前的栈状态
    int *return_addr = (int*)(buffer + 16);  // 根据栈布局计算
    printf("栈上返回地址位置: %p\n", (void*)return_addr);
    printf("当前返回地址: 0x%x\n", *return_addr);
    
    printf("\n执行缓冲区溢出攻击...\n");
    
    // 危险的strcpy - 没有长度检查
    strcpy(buffer, input);
    
    // 显示攻击后的栈状态
    printf("溢出后返回地址: 0x%x\n", *return_addr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <攻击字符串>\n", argv[0]);
        printf("\n🎯 这是一个成功的栈溢出攻击演示\n\n");
        
        printf("攻击原理:\n");
        printf("1. 使用8字节小缓冲区\n");
        printf("2. 用精心构造的字符串溢出\n");
        printf("3. 覆盖返回地址为success_function\n");
        printf("4. 函数返回时跳转到攻击者代码\n\n");
        
        printf("成功攻击示例:\n");
        printf("1. 基础攻击 - 观察崩溃\n");
        printf("   %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB\"\n", argv[0]);
        printf("\n2. 精确攻击 - 成功劫持\n");
        printf("   %s \"$(python3 -c 'import sys; print(\\\"A\\\"*24 + \\\"\\\\\\\\x60\\\\\\\\xe4\\\\\\\\x32\\\\\\\\xe4\\\\\\\\x55\\\\\\\\xb3\\\"))'\"\n", argv[0]);
        
        printf("\n📍 注意: 每次运行地址可能不同，需要动态计算\n");
        return 1;
    }
    
    printf("攻击字符串长度: %zu 字节\n", strlen(argv[1]));
    printf("攻击内容: %s\n\n", argv[1]);
    
    vulnerable_function(argv[1]);
    
    // 如果到这里说明攻击失败了
    printf("\n❌ 攻击失败 - 程序正常返回\n");
    return 0;
}
