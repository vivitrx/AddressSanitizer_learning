#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int main() {
    // 分配接近页面大小的缓冲区
    size_t page_size = getpagesize();
    char *buf = malloc(page_size - 50);  // 留50字节到页面边界
    
    // 正常填充
    for (size_t i = 0; i < page_size - 50; i++) {
        buf[i] = 'A';
    }
    
    // 越界写入到下一个页面
    buf[page_size - 1] = 'X';  // 这会跨越页面边界
    
    return 0;
}
