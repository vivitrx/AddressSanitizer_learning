/**
 * @file toy_asan.h
 * @brief Toy AddressSanitizer 公共接口定义
 * 
 * 本文件定义了玩具ASan的所有公共接口，包括：
 * - 数据结构：分配记录、内存布局
 * - 全局变量：分配表、状态标志
 * - 核心函数：内存分配、错误检测
 * 
 * 模块组成：
 * - toy_malloc.c: 核心内存分配器
 * - metadata.c: 分配记录管理
 * - globals.c: 全局变量定义
 * - signal_handler.c: SIGSEGV处理器（待实现）
 * - init.c: 初始化函数（待实现）
 * 
 * @author Toy ASan Project
 * @version 1.0
 */

#ifndef TOY_ASAN_H
#define TOY_ASAN_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

// 常量定义
#define MAX_ALLOCATIONS 1000

// 分配记录结构
struct allocation_record {
    void *base_addr;              // 整个3页块的基地址（包含保护页）
    void *user_addr;              // 用户看到的地址（中间页）
    size_t user_size;             // 用户请求的大小
    void *left_guard;             // 左保护页地址
    void *right_guard;            // 右保护页地址
    bool in_use;                 // 是否使用中
};

// 全局变量声明
extern struct allocation_record alloc_table[MAX_ALLOCATIONS];
extern int alloc_count;
extern size_t page_size;
extern bool toy_asan_initialized;

// 核心内存分配函数
void* toy_malloc(size_t size);
void toy_free(void *ptr);

// 元数据管理函数
int add_allocation(void *base, void *user, size_t user_size);
struct allocation_record* find_allocation(void *addr);
struct allocation_record* find_allocation_by_user_addr(void *user_addr);
void remove_allocation(void *user_addr);
void print_allocations(void);  // 调试用

// 内存布局计算函数
void* user_to_base(void *user_ptr);
void* base_to_user(void *base_ptr);
bool is_our_guard_page(void *addr);

// 信号处理函数
void sigsegv_handler(int sig, siginfo_t *info, void *context);
void setup_signal_handler(void);

// 初始化函数
void toy_asan_init(void);

// 辅助函数
size_t get_system_page_size(void);

// 错误报告函数
void report_buffer_overflow(void *fault_addr, struct allocation_record *rec, bool is_left_guard);

#endif // TOY_ASAN_H
