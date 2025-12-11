#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("用法: %s <输入字符串>\n", argv[0]);
    printf("这个实验演示堆缓冲区溢出如何影响相邻的堆块\n\n");

    printf("实验步骤:\n");
    printf("1. 正常长度输入: %s \"normal\"\n", argv[0]);
    printf("2. 刚好填满: %s \"exactly16bytes!!\"\n", argv[0]);
    printf("3. 轻微溢出: %s \"overflow_by_2\"\n", argv[0]);
    printf("4. 严重溢出: %s \"this_will_overflow_into_next_block\"\n", argv[0]);

    printf("\n观察要点:\n");
    printf("- buffer1和buffer2的地址距离\n");
    printf("- buffer2的内容是否被破坏\n");
    printf("- 程序是否在free时崩溃\n");
    return 1;
  }

  printf("=== 堆溢出演示 ===\n");
  printf("输入长度: %zu 字节\n", strlen(argv[1]));

  // 分配两个相邻的堆块
  char *buffer1 = malloc(16);
  char *buffer2 = malloc(16);

  if (!buffer1 || !buffer2) {
    printf("内存分配失败\n");
    return 1;
  }

  printf("buffer1地址: %p\n", (void *)buffer1);
  printf("buffer2地址: %p\n", (void *)buffer2);
  printf("距离: %ld 字节\n", (long)(buffer2 - buffer1));

  // 初始化buffer2
  strcpy(buffer2, "SECRET_DATA");
  printf("buffer2初始内容: %s\n", buffer2);

  // 向buffer1溢出写入
  printf("向buffer1写入 %zu 字节的数据...\n", strlen(argv[1]));
  strcpy(buffer1, argv[1]); // 危险：没有长度检查

  printf("溢出后buffer2内容: %s\n", buffer2);

  // 检查是否破坏了buffer2
  if (strcmp(buffer2, "SECRET_DATA") != 0) {
    printf("!!! 警告：buffer2内容被破坏了 !!!\n");
  } else {
    printf("buffer2内容完好\n");
  }

  // 释放内存
  printf("释放内存...\n");
  free(buffer1);
  free(buffer2);

  printf("程序正常结束\n");
  return 0;
}
