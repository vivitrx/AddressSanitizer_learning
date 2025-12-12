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
#include <unistd.h>      // getpid()
#include <execinfo.h>    // backtrace, backtrace_symbols

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
  sa.sa_sigaction = sigsegv_handler;     // 使用三参数处理器
  sa.sa_flags = SA_SIGINFO | SA_RESTART; // 获取详细信息 + 自动重启系统调用
  sigemptyset(&sa.sa_mask);              // 清空信号掩码

  // 注册SIGSEGV处理器
  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    perror("Toy ASan: sigaction failed");
    exit(1);
  }

  handler_installed = true;
  printf("Toy ASan: SIGSEGV handler installed\n");
}

/**
 * @brief 从si_code推断访问类型
 * @param si_code 信号的si_code字段
 * @return "READ" 或 "WRITE"
 */
const char *infer_access_type(int si_code) {
  // 根据信号代码推断访问类型
  // SEGV_ACCERR (1): 权限错误，通常是写操作
  // SEGV_ACCADI (2): 地址错误，可能是读或写
  // 其他情况默认为写操作
  switch (si_code) {
    case SEGV_ACCERR:
      return "WRITE";
    case SEGV_ACCADI:
      return "READ";  // 简化处理，实际可能更复杂
    default:
      return "WRITE";  // 默认假设为写
  }
}

/**
 * @brief 打印当前调用栈（错误发生时）
 */
void print_call_stack(void) {
  void *buffer[MAX_BACKTRACE_FRAMES];
  int frames = backtrace(buffer, MAX_BACKTRACE_FRAMES);
  char **symbols = backtrace_symbols(buffer, frames);
  
  // 跳过信号处理器本身，从第1帧开始
  for (int i = 1; i < frames; i++) {
    printf("    #%d %p\n", i-1, buffer[i]);
  }
  
  free(symbols);
}

/**
 * @brief 打印内存位置关系
 * @param fault_addr 故障地址
 * @param rec 分配记录
 */
void print_memory_relation(void *fault_addr, struct allocation_record *rec) {
  bool is_left_overflow = (fault_addr < rec->user_addr);
  const char *direction = is_left_overflow ? "left" : "right";
  size_t distance;
  void *region_start, *region_end;
  
  if (is_left_overflow) {
    distance = (char*)rec->user_addr - (char*)fault_addr;
    region_start = rec->user_addr;
    region_end = (char*)rec->user_addr + rec->user_size;
  } else {
    distance = (char*)fault_addr - ((char*)rec->user_addr + rec->user_size);
    region_start = rec->user_addr;
    region_end = (char*)rec->user_addr + rec->user_size;
  }
  
  printf("%p is located %zu bytes to %s of %zu-byte region [%p,%p)\n",
         fault_addr, distance, direction, rec->user_size, region_start, region_end);
}

/**
 * @brief 打印分配位置信息
 * @param rec 分配记录（需要扩展结构）
 */
void print_allocation_location(struct allocation_record *rec) {
  // 如果有分配位置信息
  if (rec->alloc_backtrace_size > 0) {
    printf("allocated by thread T0 here:\n");
    
    for (int i = 0; i < rec->alloc_backtrace_size; i++) {
      printf("    #%d %p\n", i, rec->alloc_backtrace[i]);
    }
  }
}

/**
 * @brief 转发给默认处理器
 * @param sig 信号编号
 * @param info 信号信息
 */
void forward_to_default_handler(int sig, siginfo_t *info) {
  signal(sig, SIG_DFL);
  raise(sig);
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
 */
void sigsegv_handler(int sig, siginfo_t *info, void *context) {
  if (sig != SIGSEGV) {
    return;
  }

  void *fault_addr = info->si_addr;
  struct allocation_record *rec = find_allocation(fault_addr);
  
  if (!rec) {
    // 不是我们的保护页，转发给默认处理器
    forward_to_default_handler(sig, info);
    return;
  }

  // =================== 1. 错误头部信息 ==================
  printf("=================================================================\n");
  printf("==%d==ERROR: Toy AddressSanitizer: heap-buffer-overflow on address %p\n", getpid(), fault_addr);

  // =================== 2. 访问信息详情 ==================
  const char *access_type = infer_access_type(info->si_code);
  printf("%s of size 1 at %p thread T0\n", access_type, fault_addr);

  // =================== 3. 当前调用栈 ==================
  print_call_stack();

  printf("\n");
  
  // =================== 4. 内存位置关系 ==================
  print_memory_relation(fault_addr, rec);
  
  printf("\n");
  
  // =================== 5. 分配位置跟踪 ==================
  print_allocation_location(rec);

  // =================== 6. 错误摘要 ==================
  printf("SUMMARY: Toy AddressSanitizer: heap-buffer-overflow in main\n");
  
  printf("=================================================================\n");
  exit(1);
}
