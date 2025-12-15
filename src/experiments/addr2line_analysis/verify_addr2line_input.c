#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void analyze_elf_layout() {
    printf("=== ELF Layout Analysis ===\n");
    
    // 1. 符号表分析
    printf("\n1. Symbol Table Analysis:\n");
    printf("   objdump -t signal_handler_test | grep -E '(main|toy_malloc)'\n");
    system("objdump -t signal_handler_test | grep -E '(main|toy_malloc)'");
    
    // 2. Section信息
    printf("\n2. Section Information:\n");
    printf("   readelf -S signal_handler_test | grep -E 'Name|\\.(text|data)'\n");
    system("readelf -S signal_handler_test | grep -E 'Name|\\.(text|data)'");
    
    // 3. Program Headers
    printf("\n3. Program Headers (LOAD segments):\n");
    printf("   readelf -l signal_handler_test | grep LOAD\n");
    system("readelf -l signal_handler_test | grep LOAD");
}

void test_addr2line_inputs() {
    printf("\n=== Addr2line Input Testing ===\n");
    
    // 获取符号表地址
    printf("\nTesting different address inputs to addr2line:\n");
    
    // 测试1：符号表地址
    printf("\n1. Testing symbol table addresses:\n");
    printf("   addr2line -fi -e signal_handler_test 0x2509 (main symbol address)\n");
    system("addr2line -fi -e signal_handler_test 0x2509");
    printf("   addr2line -fi -e signal_handler_test 0x3534 (toy_malloc symbol address)\n");
    system("addr2line -fi -e signal_handler_test 0x3534");
    
    // 测试2：相对于section的地址（减去.text section偏移）
    printf("\n2. Testing section-relative addresses:\n");
    // 需要先获取.text section的偏移
    printf("   (Need to get .text section offset first)\n");
    
    // 测试3：我们公式计算的地址
    printf("\n3. Testing our calculated addresses:\n");
    printf("   addr2line -fi -e signal_handler_test 0x25f2 (our formula result for main)\n");
    system("addr2line -fi -e signal_handler_test 0x25f2");
    printf("   addr2line -fi -e signal_handler_test 0x36d7 (our formula result for toy_malloc)\n");
    system("addr2line -fi -e signal_handler_test 0x36d7");
}

void analyze_maps_info() {
    printf("\n=== Runtime Maps Analysis ===\n");
    
    // 运行时分析
    printf("\n1. Runtime mapping information:\n");
    system("./analyze_maps");
    
    printf("\n2. Maps filtering for signal_handler_test:\n");
    printf("   (Need to run signal_handler_test and capture its maps)\n");
}

int main() {
    printf("Addr2line Input Type Verification Experiment\n");
    printf("=====================================\n");
    
    // 分析ELF布局
    analyze_elf_layout();
    
    // 测试不同的addr2line输入
    test_addr2line_inputs();
    
    // 分析运行时maps
    analyze_maps_info();
    
    printf("\n=== Conclusion ===\n");
    printf("Based on the above tests, we can determine whether addr2line expects:\n");
    printf("A. Address relative to ELF file start\n");
    printf("B. Address relative to section start\n");
    printf("C. Something else\n");
    
    return 0;
}
