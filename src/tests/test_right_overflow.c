#include "../toy_asan/toy_asan.h"
#include <stdio.h>
#include <string.h>

int main() {
    toy_asan_init();
    
    char *buf = toy_malloc(100);
    printf("User buffer: %p\n", buf);
    printf("User size: 100\n");
    
    // 计算右保护页的位置
    // 内存布局: [左保护页][用户数据][右保护页]
    // 用户数据在页中间，右保护页在下一页开始处
    size_t page_size = 4096;  // 从调试输出看到的
    void *user_page_start = (void*)((uintptr_t)buf & ~(page_size - 1));  // 用户页开始
    void *right_guard = (char*)user_page_start + page_size;  // 右保护页开始
    
    printf("User page start: %p\n", user_page_start);
    printf("Right guard page: %p\n", right_guard);
    printf("Distance from buf to right guard: %ld bytes\n", (char*)right_guard - (char*)buf);
    
    // 尝试访问右保护页
    printf("Trying to write to right guard page...\n");
    *(char*)right_guard = 'X';
    
    printf("No SIGSEGV triggered!\n");
    return 0;
}
