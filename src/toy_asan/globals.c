/**
 * @file globals.c
 * @brief Toy AddressSanitizer 全局变量定义和辅助函数
 * 
 * 本文件定义了ASan系统所需的所有全局变量，包括：
 * - alloc_table: 分配记录数组，跟踪所有内存分配
 * - alloc_count: 当前分配数量计数器
 * - page_size: 系统页面大小缓存
 * - toy_asan_initialized: 系统初始化标志
 * 
 * 同时提供辅助函数：
 * - get_page_size(): 获取并缓存系统页面大小
 * 
 * 全局变量的生命周期：
 * - 程序启动时自动初始化为0
 * - toy_asan_init()中完成完整初始化
 * - 程序结束时自动清理
 * 
 * @author Toy ASan Project
 * @version 1.0
 */

#include "toy_asan.h"
#include <unistd.h>
#include <stdio.h>

// 全局变量定义
struct allocation_record alloc_table[MAX_ALLOCATIONS];
int alloc_count = 0;
size_t page_size = 0;
bool toy_asan_initialized = false;

// 获取页面大小的辅助函数
size_t get_system_page_size(void) {
    if (page_size == 0) {
        page_size = getpagesize();
        printf("Toy ASan: page size = %zu bytes\n", page_size);
    }
    return page_size;
}
