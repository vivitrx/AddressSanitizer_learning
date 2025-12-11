/**
 * @file init.c
 * @brief Toy AddressSanitizer 初始化实现
 * 
 * 本文件实现ASan系统的初始化功能，确保在首次使用
 * 前正确设置信号处理器和全局状态。
 * 
 * 核心功能：
 * - toy_asan_init(): 系统初始化入口
 * - 设置信号处理器
 * - 初始化全局状态
 * 
 * @author Toy ASan Project
 * @version 1.0
 */

#include "toy_asan.h"
#include <stdio.h>

/**
 * @brief 初始化Toy AddressSanitizer系统
 * 
 * 执行系统初始化的必要步骤：
 * 1. 安装SIGSEGV信号处理器
 * 2. 标记系统为已初始化状态
 * 
 * 调用时机：
 * - toy_malloc()首次分配时懒加载
 * - 程序启动时显式调用
 * 
 * @note
 * - 重复调用是安全的，会检查初始化状态
 * - 初始化失败时程序无法继续运行
 */
void toy_asan_init(void) {
    if (toy_asan_initialized) {
        return; // 已经初始化过了
    }
    
    printf("Toy ASan: Initializing...\n");
    
    // 安装信号处理器
    setup_signal_handler();
    
    // 标记为已初始化
    toy_asan_initialized = true;
    
    printf("Toy ASan: Initialization complete\n");
}
