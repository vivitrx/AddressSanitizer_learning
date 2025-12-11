#include <stdio.h>
#include <string.h>

void vulnerable_function(char *input) {
    char buffer[16];  // 只有16字节的缓冲区
    printf("buffer地址: %p\n", (void*)buffer);
    printf("input地址:  %p\n", (void*)input);
    
    // 危险：没有长度检查的复制
    strcpy(buffer, input);
    
    printf("复制完成，buffer内容: %s\n", buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: %s <输入字符串>\n", argv[0]);
        printf("试试不同长度的输入观察效果\n");
        printf("例如:\n");
        printf("  %s \"short\"                    # 正常情况\n", argv[0]);
        printf("  %s \"exactly16bytes!!!\"         # 刚好16字节\n", argv[0]);
        printf("  %s \"this_is_way_too_long\"      # 溢出\n", argv[0]);
        printf("  %s \"AAAAAAAAAAAAAAAAAAAAAAAA\"  # 严重溢出\n", argv[0]);
        return 1;
    }
    
    printf("=== 栈溢出演示 ===\n");
    printf("输入长度: %zu 字节\n", strlen(argv[1]));
    printf("缓冲区大小: 16 字节\n");
    
    vulnerable_function(argv[1]);
    
    printf("程序正常结束\n");
    return 0;
}
