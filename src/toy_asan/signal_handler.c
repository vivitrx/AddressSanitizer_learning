/**
 * @file signal_handler.c
 * @brief Toy AddressSanitizer 信号处理器实现
 * 
 * 本文件实现SIGSEGV信号的捕获和处理，这是ASan检测
 * 内存溢出的核心机制。通过sigaction注册处理器，
 * 当用户访问保护页时捕获信号并报告错误。
 * 
 * 核心功能：
 * - setup_signal_handler(): 注册SIGSEGV处理器
 * - sigsegv_handler(): 处理保护页访问信号
 * 
 * 关键标志：
 * - SA_SIGINFO: 获取详细的信号信息（包括故障地址）
 * - SA_RESTART: 确保被中断的系统调用自动重启
 * 
 * @author Toy ASan Project
 * @version 1.0
 */

#include "toy_asan.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 注册SIGSEGV信号处理器
 * 
 * 使用sigaction系统调用注册sigsegv_handler作为SIGSEGV的处理器。
 * 配置SA_SIGINFO标志以获取详细的信号信息（包括故障地址），
 * 配置SA_RESTART标志以确保被中断的系统调用自动重启。
 * 
 * 调用时机：
 * - toy_asan_init()中调用
 * - toy_malloc()首次分配时懒加载
 * 
 * 错误处理：
 * - 注册失败时打印错误信息并退出程序
 * - 因为无法捕获保护页访问，ASan无法正常工作
 * 
 * @note
 * - 使用静态变量防止重复注册
 * - 清空信号掩码确保处理时不被其他信号干扰
 */
void setup_signal_handler(void) {
    static bool handler_installed = false;
    
    if (handler_installed) {
        return; // 防止重复安装
    }
    
    struct sigaction sa;
    
    // 配置信号处理结构
    sa.sa_sigaction = sigsegv_handler;           // 使用三参数处理器
    sa.sa_flags = SA_SIGINFO | SA_RESTART;      // 获取详细信息 + 自动重启系统调用
    sigemptyset(&sa.sa_mask);                    // 清空信号掩码
    
    // 注册SIGSEGV处理器
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Toy ASan: sigaction failed");
        exit(1);
    }
    
    handler_installed = true;
    printf("Toy ASan: SIGSEGV handler installed\n");
}

/**
 * @brief SIGSEGV信号处理器
 * @param sig 信号编号（应为SIGSEGV = 11）
 * @param info 包含信号详细信息的结构体
 * @param context 处理器上下文（寄存器状态等）
 * 
 * 处理流程：
 * 1. 从siginfo_t获取故障地址
 * 2. 调用find_allocation()检测是否为我们的保护页
 * 3. 如果是保护页访问，报告溢出并退出
 * 4. 如果不是保护页访问，转发给默认处理器
 * 
 * @note
 * - 目前只实现基础检测，不报告详细错误信息
 * - 后续版本将添加report_buffer_overflow()调用
 */
void sigsegv_handler(int sig, siginfo_t *info, void *context) {
    if (sig != SIGSEGV) {
        return;
    }
    
    void *fault_addr = info->si_addr;
    printf("Toy ASan: SIGSEGV at address %p\n", fault_addr);
    
    // 检查是否是我们的保护页访问
    struct allocation_record *rec = find_allocation(fault_addr);
    if (rec) {
        printf("Toy ASan: Buffer overflow detected in our allocation\n");
        
        // TODO: 调用report_buffer_overflow()进行详细报告
        // TODO: 区分左溢出和右溢出
        
        exit(1);
    }
    
    // 不是我们的保护页访问，可能是正常的程序错误
    printf("Toy ASan: Not our guard page, forwarding to default handler\n");
    
    // 恢复默认处理器并重新触发信号
    signal(SIGSEGV, SIG_DFL);
    raise(SIGSEGV);
}
