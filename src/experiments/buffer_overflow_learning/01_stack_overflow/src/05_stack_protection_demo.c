#include <stdio.h>
#include <string.h>

void success_function() {
    printf("🎯 成功劫持控制流！\n");
    printf("🎯 栈溢出攻击成功！\n");
}

void vulnerable_function(char *input) {
    char buffer[16];  // 缓冲区
    
    printf("=== 栈保护对比演示 ===\n");
    printf("buffer地址: %p\n", (void*)buffer);
    printf("输入长度: %zu 字节\n", strlen(input));
    
    // 危险的缓冲区溢出
    strcpy(buffer, input);
    
    printf("strcpy执行完成\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("栈保护对比演示\n\n");
        
        printf("编译选项对比:\n");
        printf("❌ 无栈保护: cmake -DENABLE_STACK_PROTECTION=OFF ..\n");
        printf("✅ 有栈保护: cmake -DENABLE_STACK_PROTECTION=ON ..\n\n");
        
        printf("测试用例:\n");
        printf("1. 正常输入: %s \"12345678\"\n", argv[0]);
        printf("2. 轻微溢出: %s \"12345678901234567890\"\n", argv[0]);
        printf("3. 严重溢出: %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n", argv[0]);
        printf("4. 极端溢出: %s \"$(python3 -c 'print(\"A\"*100)')\"\n", argv[0]);
        
        return 1;
    }
    
    printf("开始测试...\n\n");
    
    vulnerable_function(argv[1]);
    
    printf("✅ 程序正常结束 - 栈保护生效了！\n");
    return 0;
}
