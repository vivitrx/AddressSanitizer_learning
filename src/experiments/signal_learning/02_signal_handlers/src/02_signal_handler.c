/**
 * @file 02_signal_handler.c
 * @brief 信号处理器实验 - 学习signal() vs sigaction()的区别
 *
 * 本实验演示：
 * - 如何注册自定义信号处理函数
 * - signal() vs sigaction()的区别
 * - 信号处理的安全性考虑
 * - 信号处理器的返回行为
 *
 * 编译运行：
 * gcc -o 02_signal_handler 02_signal_handler.c && ./02_signal_handler
 *
 * @author Signal Learning Project
 * @version 1.0
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 全局变量用于跟踪信号处理次数
volatile int signal_count = 0;
volatile sig_atomic_t safe_counter = 0;

// 简单的signal()处理器（基础版本）
void simple_handler(int sig) {
  printf("🔔 简单处理器收到信号: %d (%s)\n", sig, strsignal(sig));
  signal_count++; // 注意：这不是线程安全的

  // 尝试重新注册信号处理器（signal()的常见问题）
  signal(sig, simple_handler);
  printf("   已重新注册信号处理器\n");
}

// 安全的signal()处理器（改进版本）
void safe_signal_handler(int sig) {
  // 使用sig_atomic_t类型的变量，这是信号安全的
  safe_counter++;

  // 在信号处理器中只使用异步信号安全的函数
  // printf()实际上不是信号安全的，但我们为了演示使用
  write(STDOUT_FILENO, "🛡️ 安全处理器收到信号\n", 25);

  // 重新注册处理器（某些系统需要）
  signal(sig, safe_signal_handler);
}

// sigaction()处理器（高级版本）
struct sigaction old_action;
void advanced_handler(int sig) {
  printf("⚡ 高级处理器收到信号: %d (%s)\n", sig, strsignal(sig));
  printf("   处理器地址: %p\n", advanced_handler);

  // 显示信号的详细信息（如果可用）
  if (sig == SIGSEGV) {
    printf("   这是一个段错误信号！\n");
  } else if (sig == SIGINT) {
    printf("   用户按下Ctrl+C\n");
  } else if (sig == SIGTERM) {
    printf("   收到终止请求\n");
  }

  // sigaction()不需要重新注册
  printf("   sigaction()自动保持处理器\n");
}

// 演示信号处理器的限制
void problematic_handler(int sig) {
  printf("❌ 问题处理器收到信号: %d\n", sig);

  // 在信号处理器中调用不安全的函数（这可能导致问题）
  printf("   尝试调用malloc()...\n");
  char *buffer = malloc(100); // 危险！
  if (buffer) {
    strcpy(buffer, "在信号处理器中分配的内存");
    printf("   成功分配内存: %s\n", buffer);
    free(buffer); // 也是危险的
  }

  // 调用printf()在信号处理器中技术上是不安全的
  printf("   这个处理器可能引起竞态条件\n");
}

// 测试signal()的基本用法
void test_signal_basic(void) {
  printf("\n=== 测试1: signal()基本用法 ===\n");

  // 注册信号处理器
  if (signal(SIGINT, simple_handler) == SIG_ERR) {
    perror("signal(SIGINT) 失败");
    return;
  }

  printf("已注册SIGINT处理器 (Ctrl+C)\n");
  printf("请在5秒内按Ctrl+C发送信号...\n");

  // 等待信号
  sleep(10);

  printf("等待结束，收到 %d 个信号\n", signal_count);

  // 恢复默认处理
  signal(SIGINT, SIG_DFL);
}

// 测试signal()的可靠性问题
void test_signal_reliability(void) {
  printf("\n=== 测试2: signal()可靠性问题 ===\n");

  printf("连续发送SIGINT信号，观察信号处理器是否失效...\n");

  if (signal(SIGINT, simple_handler) == SIG_ERR) {
    perror("signal(SIGINT) 失败");
    return;
  }

  printf("已注册处理器，请快速连续按Ctrl+C 3-5次\n");
  printf("观察处理器是否能够正确处理所有信号...\n");

  sleep(10);

  printf("测试结束，总信号数: %d\n", signal_count);

  // 恢复默认处理
  signal(SIGINT, SIG_DFL);
}

// 测试sigaction()的强大功能
void test_sigaction(void) {
  printf("\n=== 测试3: sigaction()高级用法 ===\n");

  struct sigaction sa;
  sa.sa_handler = advanced_handler;
  sigemptyset(&sa.sa_mask); // 不阻塞其他信号
  sa.sa_flags = 0;          // 默认行为

  // 注册信号处理器
  if (sigaction(SIGINT, &sa, &old_action) == -1) {
    perror("sigaction(SIGINT) 失败");
    return;
  }

  printf("已使用sigaction()注册SIGINT处理器\n");
  printf("sigaction()的优势：\n");
  printf("  - 不需要重新注册处理器\n");
  printf("  - 可以控制信号掩码\n");
  printf("  - 可以设置更多标志位\n");
  printf("  - 更可靠和可移植\n");

  printf("请按Ctrl+C测试（3次机会）...\n");

  for (int i = 0; i < 3; i++) {
    printf("⏸️ 程序已暂停，等待信号发送... (第%d次)\n", i + 1);
    printf("💡 使用信号发送工具: ./build/bin/03_signal_sender %d <信号编号>\n",
           getpid());
    printf("   或在另一个终端按Ctrl+C (如果信号处理器生效)\n");
    printf("⏳ 等待信号中...\n");

    sleep(20);

    printf("✅ 第%d次信号处理完成\n\n", i + 1);
  }

  // 恢复原来的处理器
  if (sigaction(SIGINT, &old_action, NULL) == -1) {
    perror("恢复信号处理器失败");
  }
}

// 测试信号掩码和阻塞
void test_signal_masking(void) {
  printf("\n=== 测试4: 信号掩码和阻塞 ===\n");

  struct sigaction sa;
  sa.sa_handler = advanced_handler;

  // 设置信号掩码：在处理SIGINT时阻塞SIGTERM
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGTERM);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction(SIGINT) 失败");
    return;
  }

  // 设置SIGTERM的处理器
  struct sigaction sa_term;
  sa_term.sa_handler = advanced_handler;
  sigemptyset(&sa_term.sa_mask);
  sa_term.sa_flags = 0;

  if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
    perror("sigaction(SIGTERM) 失败");
    return;
  }

  printf("已设置信号掩码：处理SIGINT时阻塞SIGTERM\n");
  printf("测试步骤：\n");
  printf("1. 按Ctrl+C发送SIGINT\n");
  printf("2. 立即发送SIGTERM信号 (kill -TERM %d)\n", getpid());
  printf("3. 观察SIGTERM是否被延迟处理\n");

  printf("请测试，等待10秒...\n");
  sleep(10);

  // 恢复默认处理
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
}

// 测试不安全的信号处理器
void test_unsafe_handler(void) {
  printf("\n=== 测试5: 不安全的信号处理器 ===\n");

  printf("注册包含不安全函数的信号处理器...\n");
  printf("⚠️ 这个测试可能导致程序崩溃或死锁！\n");

  if (signal(SIGINT, problematic_handler) == SIG_ERR) {
    perror("signal(SIGINT) 失败");
    return;
  }

  printf("已注册问题处理器，请按Ctrl+C测试...\n");
  printf("观察是否出现异常行为...\n");

  sleep(8);

  // 恢复默认处理
  signal(SIGINT, SIG_DFL);
}

// 演示信号处理器的返回行为
void test_handler_return(void) {
  printf("\n=== 测试6: 信号处理器返回行为 ===\n");

  printf("测试不同信号的处理情况...\n");

  // 对于SIGSEGV，我们通常不能安全地返回
  printf("注意：SIGSEGV、SIGFPE等信号的处理器返回通常是危险的！\n");
  printf("这些信号通常表示程序状态已损坏。\n");

  // 注册一个简单的处理器来观察行为
  signal(SIGUSR1, simple_handler);
  printf("已注册SIGUSR1处理器，发送信号: kill -USR1 %d\n", getpid());

  sleep(5);

  // 恢复默认处理
  signal(SIGUSR1, SIG_DFL);
}

// 主函数
int main(int argc, char *argv[]) {
  printf("🔍 信号处理器实验 - signal() vs sigaction()\n");
  printf("进程ID: %d\n", getpid());
  printf("========================================\n");

  if (argc != 2) {
    printf("用法: %s <测试编号>\n", argv[0]);
    printf("可用的测试:\n");
    printf("  1 - signal()基本用法\n");
    printf("  2 - signal()可靠性问题\n");
    printf("  3 - sigaction()高级用法\n");
    printf("  4 - 信号掩码和阻塞\n");
    printf("  5 - 不安全的信号处理器（危险）\n");
    printf("  6 - 信号处理器返回行为\n");
    printf("\n💡 提示：大部分测试需要你手动发送信号\n");
    printf("   Ctrl+C = SIGINT\n");
    printf("   kill -TERM %d = SIGTERM\n", getpid());
    printf("   kill -USR1 %d = SIGUSR1\n", getpid());
    return 1;
  }

  int test_num = atoi(argv[1]);

  switch (test_num) {
  case 1:
    test_signal_basic();
    break;
  case 2:
    test_signal_reliability();
    break;
  case 3:
    test_sigaction();
    break;
  case 4:
    test_signal_masking();
    break;
  case 5:
    test_unsafe_handler();
    break;
  case 6:
    test_handler_return();
    break;
  default:
    printf("无效的测试编号: %d\n", test_num);
    return 1;
  }

  printf("\n实验完成！\n");
  printf("🎯 关键知识点总结：\n");
  printf("  1. signal()简单但有限制\n");
  printf("  2. sigaction()功能更强大\n");
  printf("  3. 信号处理器要使用异步信号安全的函数\n");
  printf("  4. 某些信号的处理器返回是危险的\n");
  printf("  5. 信号掩码可以控制信号的处理顺序\n");

  return 0;
}
