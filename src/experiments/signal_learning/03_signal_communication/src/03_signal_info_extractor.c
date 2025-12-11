#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

// 添加寄存器定义（如果系统中没有）
#ifndef REG_RIP
#define REG_RIP 14 // 指令指针
#define REG_RSP 19 // 栈指针
#define REG_RBP 10 // 基址指针
#endif

// 全局状态变量，便于GDB调试观察
volatile sig_atomic_t signal_received = 0;
volatile void *fault_address = NULL;
volatile intptr_t instruction_pointer = 0;
volatile int signal_code = 0;
volatile int signal_number = 0;
volatile pid_t sender_pid = 0;

// 信号安全的错误报告函数
void safe_write_error(const char *msg) {
  write(STDERR_FILENO, msg, strlen(msg));
}

// 高级信号处理器 - 三参数处理器
void advanced_siginfo_handler(int sig, siginfo_t *info, void *ucontext) {
  // 立即标记信号到达
  signal_received = 1;
  signal_number = sig;

  // 提取关键信号信息
  if (info) {
    fault_address = info->si_addr;
    signal_code = info->si_code;
    sender_pid = info->si_pid;
  }

  // 提取指令指针（从上下文）
  if (ucontext) {
    ucontext_t *uc = (ucontext_t *)ucontext;
#ifdef __x86_64__
    instruction_pointer = uc->uc_mcontext.gregs[REG_RIP];
#elif defined(__i386__)
    instruction_pointer = uc->uc_mcontext.gregs[REG_EIP];
#endif
  }

  // 信号安全的详细错误报告
  char error_msg[500];
  int len = snprintf(error_msg, sizeof(error_msg),
                     "🔍 === CRASH ANALYSIS ===\n"
                     "Signal: %d (%s)\n"
                     "Address: %p\n"
                     "Code: %d\n"
                     "PID: %d\n"
                     "Instruction: 0x%lx\n"
                     "=========================\n",
                     sig, strsignal(sig), fault_address, signal_code,
                     sender_pid, instruction_pointer);

  write(STDOUT_FILENO, error_msg, len);

  // 分析信号类型和原因
  char analysis_msg[300];
  int analysis_len = 0;

  if (sig == SIGSEGV) {
    switch (signal_code) {
    case SEGV_MAPERR:
      analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                              "📍 分析: 地址映射错误\n"
                              "   地址 %p 不在进程地址空间中\n"
                              "   可能原因: 空指针访问、无效地址\n",
                              fault_address);
      break;
    case SEGV_ACCERR:
      analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                              "📍 分析: 访问权限错误\n"
                              "   地址 %p 不可写/不可执行\n"
                              "   可能原因: 只读内存写入、代码段执行\n",
                              fault_address);
      break;
    default:
      analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                              "📍 分析: 未知段错误\n"
                              "   信号代码: %d\n",
                              signal_code);
      break;
    }
  } else if (sig == SIGBUS) {
    analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                            "📍 分析: 总线错误\n"
                            "   地址 %p 对齐问题或硬件错误\n",
                            fault_address);
  } else if (sig == SIGFPE) {
    analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                            "📍 分析: 浮点异常\n"
                            "   除零错误或溢出\n");
  } else if (sig == SIGILL) {
    analysis_len = snprintf(analysis_msg, sizeof(analysis_msg),
                            "📍 分析: 非法指令\n"
                            "   指令 0x%lx 无法执行\n",
                            instruction_pointer);
  }

  if (analysis_len > 0) {
    write(STDOUT_FILENO, analysis_msg, analysis_len);
  }

  // 调用栈信息（简化版）
  if (ucontext) {
    ucontext_t *uc = (ucontext_t *)ucontext;
#ifdef __x86_64__
    uintptr_t rbp = uc->uc_mcontext.gregs[REG_RBP];
    uintptr_t rsp = uc->uc_mcontext.gregs[REG_RSP];

    char stack_msg[200];
    int stack_len = snprintf(stack_msg, sizeof(stack_msg),
                             "📊 栈信息:\n"
                             "   RBP: 0x%lx\n"
                             "   RSP: 0x%lx\n"
                             "   可在GDB中使用: x/32x 0x%lx\n",
                             rbp, rsp, rbp);

    write(STDOUT_FILENO, stack_msg, stack_len);
#endif
  }

  // ASan应用示例
  char asan_msg[300];
  int asan_len = snprintf(asan_msg, sizeof(asan_msg),
                          "🚀 ASan应用示例:\n"
                          "   检查是否在保护区域: is_redzone(%p)\n"
                          "   检查是否在堆区域: is_heap_address(%p)\n"
                          "   生成错误报告: generate_asan_report(%p, 0x%lx)\n"
                          "=========================\n",
                          fault_address, instruction_pointer);

  write(STDOUT_FILENO, asan_msg, asan_len);
}

// 主函数 - 设置信号处理器并等待
int main(int argc, char *argv[]) {
  struct sigaction sa;

  printf("🎯 信号信息提取器实验\n");
  printf("目标：从信号中获取详细的错误信息\n");
  printf("=====================================\n");

  // 配置高级信号处理
  sa.sa_sigaction = advanced_siginfo_handler;
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  sigemptyset(&sa.sa_mask);

  // 注册多个信号处理器
  const int signals[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGTERM};
  const char *signal_names[] = {"SIGSEGV", "SIGBUS", "SIGFPE", "SIGILL",
                                "SIGTERM"};
  const int signal_count = sizeof(signals) / sizeof(signals[0]);

  for (int i = 0; i < signal_count; i++) {
    if (sigaction(signals[i], &sa, NULL) == -1) {
      char error_msg[100];
      int len = snprintf(error_msg, sizeof(error_msg), "❌ 注册%s失败: %s\n",
                         signal_names[i], strerror(errno));
      write(STDERR_FILENO, error_msg, len);
      return 1;
    }
    printf("✅ %s处理器注册成功\n", signal_names[i]);
  }

  printf("\n📊 调试信息:\n");
  printf("   进程ID: %d\n", getpid());
  printf("   signal_received地址: %p\n", (void *)&signal_received);
  printf("   fault_address地址: %p\n", (void *)&fault_address);
  printf("   instruction_pointer地址: %p\n", (void *)&instruction_pointer);
  printf("   advanced_siginfo_handler地址: %p\n", advanced_siginfo_handler);

  printf("\n🎯 测试方法:\n");
  printf("   方法1: kill -SEGV %d\n", getpid());
  printf("   方法2: kill -BUS %d\n", getpid());
  printf("   方法3: kill -FPE %d\n", getpid());
  printf("   方法4: kill -ILL %d\n", getpid());
  printf("   方法5: kill -TERM %d\n", getpid());
  printf("   方法6: GDB调试，然后signal <信号编号>\n");

  printf("\n⏳ 等待信号...\n");

  // 主循环 - 等待信号
  int wait_count = 0;
  while (!signal_received) {
    sleep(1);
    wait_count++;

    // 每10秒显示一次等待状态
    if (wait_count % 10 == 0) {
      char status_msg[100];
      int len = snprintf(status_msg, sizeof(status_msg),
                         "⏰ 已等待%d秒，信号仍未到达...\n", wait_count);
      write(STDOUT_FILENO, status_msg, len);
    }
  }

  printf("\n🎉 信号信息提取完成！\n");
  printf("=====================================\n");
  printf("📊 最终状态:\n");
  printf("   signal_received: %d\n", signal_received);
  printf("   signal_number: %d (%s)\n", signal_number,
         strsignal(signal_number));
  printf("   fault_address: %p\n", fault_address);
  printf("   signal_code: %d\n", signal_code);
  printf("   instruction_pointer: 0x%lx\n", instruction_pointer);
  printf("   sender_pid: %d\n", sender_pid);
  printf("=====================================\n");

  // GDB调试提示
  printf("\n🔍 GDB调试命令:\n");
  printf("   (gdb) break advanced_siginfo_handler\n");
  printf("   (gdb) run\n");
  printf("   (gdb) signal 11  # 发送SIGSEGV\n");
  printf("   (gdb) print fault_address\n");
  printf("   (gdb) print instruction_pointer\n");
  printf("   (gdb) info registers\n");

  return 0;
}
