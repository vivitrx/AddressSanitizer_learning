/**
 * @file toy_malloc.c
 * @brief Toy AddressSanitizer 核心内存分配器实现
 *
 * 本文件实现了具有保护页机制的内存分配器，通过mmap+mprotect
 * 创建[保护页][用户数据][保护页]的三页内存布局：
 *
 * 内存布局：
 * ┌─────────────────────────────────────────────┐
 * │     保护页           │ PROT_NONE  ← 检测左溢出
 * ├─────────────────────────────────────────────┤
 * │     用户数据          │ RW        ← 用户可访问
 * ├─────────────────────────────────────────────┤
 * │     保护页           │ PROT_NONE  ← 检测右溢出
 * └─────────────────────────────────────────────┘
 *
 * 主要功能：
 * - toy_malloc(): 分配带保护页的内存
 * - toy_free(): 释放整个内存块
 *
 * 依赖：
 * - metadata.c: 分配记录管理
 * - globals.c: 全局变量和页面大小
 * - signal_handler.c: 错误检测（间接）
 *
 * @author Toy ASan Project
 * @version 1.0
 */

#include "toy_asan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <execinfo.h>  // 新增：backtrace支持

/**
 * @brief 分配具有保护页的内存块
 * @param size 用户请求的内存大小（字节）
 * @return 用户可访问的内存地址，失败返回NULL
 * @example
 * ```c
 * char *buf = toy_malloc(100);
 * strcpy(buf, "hello world");
 * buf[99] = 'x';     // 安全：在用户数据范围内
 * buf[-1] = 'x';     // 危险：触发左保护页SIGSEGV
 * buf[200] = 'x';    // 危险：触发右保护页SIGSEGV
 * toy_free(buf);
 * ```
 * @note
 * - 实际分配3页内存：[保护页][用户数据][保护页]
 * - 用户只能访问中间页，保护页访问会触发SIGSEGV
 * - 使用mmap+mprotect实现页面级保护，零运行时开销
 * - 分配失败时返回NULL，不设置errno
 */
void *toy_malloc(size_t size) {
  // 确保已初始化
  if (!toy_asan_initialized) {
    toy_asan_init();
  }

  // 确保页面大小已获取
  size_t ps = get_system_page_size();

  // 分配3页内存：[保护页][用户数据][保护页]
  size_t total_size = 3 * ps;
  void *base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (base_addr == MAP_FAILED) {
    perror("mmap failed");
    return NULL;
  }

  // 设置左边保护页为不可访问
  if (mprotect(base_addr, ps, PROT_NONE) != 0) {
    perror("mprotect left guard failed");
    munmap(base_addr, total_size);
    return NULL;
  }

  // 设置右边保护页为不可访问
  void *right_guard = (char *)base_addr + 2 * ps;
  if (mprotect(right_guard, ps, PROT_NONE) != 0) {
    perror("mprotect right guard failed");
    munmap(base_addr, total_size);
    return NULL;
  }

  // 计算用户看到的地址（中间页）
  void *user_addr = (char *)base_addr + ps;

  // 记录分配信息
  int slot = add_allocation(base_addr, user_addr, size);
  if (slot == -1) {
    printf("Error: allocation table full\n");
    munmap(base_addr, total_size);
    return NULL;
  }

  // 关键：记录分配时的调用栈
  struct allocation_record *rec = find_allocation_by_user_addr(user_addr);
  if (rec) {
    rec->alloc_backtrace_size = backtrace(rec->alloc_backtrace, MAX_ALLOC_BACKTRACE);
    
    // 调试输出（可选）
    if (rec->alloc_backtrace_size > 0) {
      printf("DEBUG: Allocation stack recorded with %d frames\n", rec->alloc_backtrace_size);
    }
  }

  printf("toy_malloc: allocated %zu bytes at %p (base: %p)\n", size, user_addr,
         base_addr);

  return user_addr;
}

/**
 * @brief 释放toy_malloc分配的内存
 * @param ptr toy_malloc返回的用户地址，可以为NULL
 * @example
 * ```c
 * char *buf = toy_malloc(100);
 * strcpy(buf, "hello");
 * // 使用内存...
 * toy_free(buf);     // 释放整个3页内存块
 * toy_free(NULL);     // 安全：什么都不做
 * toy_free((void*)0x123); // 警告：不是toy_malloc分配的
 * ```
 * @note
 * - 释放整个3页内存块，包括保护页和用户数据
 * - 如果地址不是toy_malloc分配的，会发出警告但不崩溃
 * - 重复free同一地址是安全的（会警告但不崩溃）
 * - free(NULL)是完全安全的，符合标准库行为
 */
void toy_free(void *usr_addr) {
  if (!usr_addr) {
    return; // free(NULL)是合法的
  }

  if (!toy_asan_initialized) {
    toy_asan_init();
  }

  // 查找分配记录
  struct allocation_record *rec = find_allocation_by_user_addr(usr_addr);
  if (!rec) {
    printf("toy_free: warning - %p not found in allocation table\n", usr_addr);
    return; // 不是我们分配的，不处理
  }

  printf("toy_free: freeing %p (base: %p)\n", usr_addr, rec->base_addr);

  // 移除分配记录
  remove_allocation(usr_addr);

  // 释放整个内存块
  size_t ps = get_system_page_size();
  size_t total_size = 3 * ps;
  if (munmap(rec->base_addr, total_size) != 0) {
    perror("munmap failed");
  }
}
