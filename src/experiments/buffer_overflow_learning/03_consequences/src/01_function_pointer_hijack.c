#include <stdio.h>
#include <string.h>

void legitimate_function() {
    printf("这是合法函数 - 正常执行路径\n");
}

void malicious_function() {
    printf("!!! 恶意函数被调用了 !!!\n");
    printf("!!! 缓冲区溢出成功劫持了函数指针 !!!\n");
    printf("!!! 攻击者现在可以执行任意代码 !!!\n");
}

struct vulnerable_struct {
    char buffer[32];
    void (*func_ptr)(void);
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        printf("这个实验演示如何通过缓冲区溢出劫持函数指针\n\n");
        
        printf("实验步骤:\n");
        printf("1. 正常调用: %s \"short\"\n", argv[0]);
        printf("2. 尝试劫持: %s \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB\"\n", argv[0]);
        
        printf("\n结构体布局:\n");
        printf("struct {\n");
        printf("    char buffer[32];    // 偏移 0\n");
        printf("    void (*func_ptr);   // 偏移 32 (可能对齐到 36 或 40)\n");
        printf("};\n");
        
        printf("\n观察要点:\n");
        printf("- func_ptr的地址和值变化\n");
        printf("- 是否调用了malicious_function\n");
        printf("- 程序控制流是否被改变\n");
        return 1;
    }
    
    printf("=== 函数指针劫持演示 ===\n");
    printf("输入长度: %zu 字节\n", strlen(argv[1]));
    
    struct vulnerable_struct data;
    
    // 初始化
    data.func_ptr = legitimate_function;
    printf("buffer地址:    %p\n", (void*)data.buffer);
    printf("func_ptr地址:  %p\n", (void*)&data.func_ptr);
    printf("func_ptr值:    %p\n", (void*)data.func_ptr);
    printf("legitimate_function地址: %p\n", (void*)legitimate_function);
    printf("malicious_function地址:  %p\n", (void*)malicious_function);
    
    // 正常调用
    printf("\n第一次调用函数指针:\n");
    data.func_ptr();
    
    // 缓冲区溢出，覆盖函数指针
    printf("\n执行缓冲区溢出...\n");
    strcpy(data.buffer, argv[1]);
    
    printf("溢出后func_ptr值: %p\n", (void*)data.func_ptr);
    
    // 检查是否被劫持
    if (data.func_ptr == malicious_function) {
        printf("!!! 警告：函数指针已被劫持到malicious_function !!!\n");
    } else if (data.func_ptr != legitimate_function) {
        printf("!!! 警告：函数指针已被修改为未知地址 !!!\n");
    } else {
        printf("函数指针未被修改\n");
    }
    
    // 再次调用，可能会调用恶意函数
    printf("\n第二次调用函数指针:\n");
    data.func_ptr();
    
    return 0;
}
