#include "src/toy_asan/toy_asan.h"
#include <stdio.h>

void test_function() {
    printf("This is a test function\n");
}

int main() {
    printf("Testing addr2line symbol resolution...\n");
    
    // 测试resolve_symbol函数
    char symbol[512];
    void *addr = (void*)test_function;
    
    printf("Testing address: %p\n", addr);
    
    if (resolve_symbol(addr, symbol, sizeof(symbol)) == 0) {
        printf("Resolved symbol: %s\n", symbol);
    } else {
        printf("Failed to resolve symbol\n");
    }
    
    return 0;
}
