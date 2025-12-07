/**
 * @file metadata.c
 * @brief Toy AddressSanitizer 分配记录管理实现
 * 
 * 本文件实现了内存分配记录的管理功能，包括：
 * - 分配记录的添加、查找、删除
 * - 保护页地址范围检测
 * - 分配状态调试输出
 * 
 * 核心数据结构：
 * - alloc_table[MAX_ALLOCATIONS]: 全局分配表
 * - 每个记录包含：基地址、用户地址、保护页信息
 * 
 * 关键功能：
 * - add_allocation(): 添加新分配记录
 * - find_allocation(): 通过地址查找记录（信号处理器使用）
 * - find_allocation_by_user_addr(): 通过用户地址查找记录（free使用）
 * - remove_allocation(): 标记记录为未使用
 * 
 * @author Toy ASan Project
 * @version 1.0
 */

#include "toy_asan.h"
#include <stdio.h>

/**
 * @brief 添加分配记录到表中
 * @param base 整个3页块的基地址
 * @param user 用户看到的地址（中间页）
 * @param user_size 用户请求的大小
 * @return 分配的槽位索引，失败返回-1
 * 
 * 将新分配的内存块信息记录到全局分配表中，用于后续的
 * 错误检测和释放操作
 */
int add_allocation(void *base, void *user, size_t user_size) {
    // 查找空闲槽位
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!alloc_table[i].in_use) {
            // 填充记录
            alloc_table[i].base_addr = base;
            alloc_table[i].user_addr = user;
            alloc_table[i].user_size = user_size;
            alloc_table[i].left_guard = base;
            alloc_table[i].right_guard = (char*)base + 2 * get_system_page_size();
            alloc_table[i].in_use = true;
            
            alloc_count++;
            printf("Added allocation at slot %d: base=%p, user=%p, size=%zu\n", 
                   i, base, user, user_size);
            return i;
        }
    }
    
    printf("Error: allocation table full\n");
    return -1; // 表满了
}

/**
 * @brief 通过地址查找分配记录（用于信号处理器）
 * @param addr 发生段错误的地址
 * @return 找到的分配记录指针，如果没找到返回NULL
 * 
 * 检查给定地址是否在任何保护页范围内：
 * - 左保护页：[base_addr, base_addr + page_size)
 * - 右保护页：[base_addr + 2*page_size, base_addr + 3*page_size)
 */
struct allocation_record* find_allocation(void *addr) {
    size_t ps = get_system_page_size();
    
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!alloc_table[i].in_use) continue;
        
        void *base = alloc_table[i].base_addr;
        
        // 检查左保护页范围：[base, base + page_size)
        if (addr >= base && addr < (char*)base + ps) {
            printf("Found left guard access: %p in [%p, %p)\n", 
                   addr, base, (char*)base + ps);
            return &alloc_table[i];
        }
        
        // 检查右保护页范围：[base + 2*page_size, base + 3*page_size)
        void *right_start = (char*)base + 2 * ps;
        if (addr >= right_start && addr < (char*)right_start + ps) {
            printf("Found right guard access: %p in [%p, %p)\n", 
                   addr, right_start, (char*)right_start + ps);
            return &alloc_table[i];
        }
    }
    return NULL; // 没找到
}

/**
 * @brief 通过用户地址查找分配记录（用于free）
 * @param user_addr 用户看到的地址
 * @return 找到的分配记录指针，如果没找到返回NULL
 * 
 * 在free操作中，用户提供的是可见的用户地址，需要找到
 * 对应的分配记录以获取完整的内存块信息
 */
struct allocation_record* find_allocation_by_user_addr(void *user_addr) {
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (alloc_table[i].in_use && alloc_table[i].user_addr == user_addr) {
            return &alloc_table[i];
        }
    }
    return NULL; // 没找到
}

/**
 * @brief 移除分配记录
 * @param user_addr 用户看到的地址
 * 
 * 将指定分配记录标记为未使用，通常在free操作中调用
 */
void remove_allocation(void *user_addr) {
    struct allocation_record *rec = find_allocation_by_user_addr(user_addr);
    if (rec) {
        rec->in_use = false;
        alloc_count--;
        printf("Removed allocation: user=%p\n", user_addr);
    } else {
        printf("Warning: tried to remove non-existent allocation: %p\n", user_addr);
    }
}

// 判断地址是否为我们的保护页
bool is_our_guard_page(void *addr) {
    struct allocation_record *rec = find_allocation(addr);
    return rec != NULL;
}

// 内存布局计算函数
void* user_to_base(void *user_ptr) {
    struct allocation_record *rec = find_allocation_by_user_addr(user_ptr);
    return rec ? rec->base_addr : NULL;
}

void* base_to_user(void *base_ptr) {
    // 基地址 + 一个页面 = 用户地址
    return (char*)base_ptr + get_system_page_size();
}

// 调试函数：打印所有分配记录
void print_allocations(void) {
    printf("=== Current Allocations (%d total) ===\n", alloc_count);
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (alloc_table[i].in_use) {
            printf("Slot %d: base=%p, user=%p, size=%zu, left_guard=%p, right_guard=%p\n",
                   i, 
                   alloc_table[i].base_addr,
                   alloc_table[i].user_addr,
                   alloc_table[i].user_size,
                   alloc_table[i].left_guard,
                   alloc_table[i].right_guard);
        }
    }
    printf("=====================================\n");
}
