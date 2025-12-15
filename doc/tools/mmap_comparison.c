#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

void file_mapping_demo() {
    printf("\n=== 文件映射演示 ===\n");
    
    // 创建并写入测试文件
    system("echo 'Original file content' > test_file.txt");
    
    int fd = open("test_file.txt", O_RDWR);
    if (fd == -1) {
        perror("open failed");
        return;
    }
    
    off_t size = lseek(fd, 0, SEEK_END);
    void *ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    printf("文件映射地址: %p\n", ptr);
    printf("初始内容: %s\n", (char*)ptr);
    
    // 修改内存会同步到文件
    strcpy((char*)ptr, "Modified through mmap");
    msync(ptr, size, MS_SYNC);
    
    printf("系统命令查看文件: ");
    system("cat test_file.txt");
    
    munmap(ptr, size);
    close(fd);
}

void anonymous_mapping_demo() {
    printf("\n=== 匿名映射演示 ===\n");
    
    size_t size = 4096;
    void *ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, 
                    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    
    printf("匿名映射地址: %p\n", ptr);
    strcpy((char*)ptr, "This is anonymous memory");
    printf("内存内容: %s\n", (char*)ptr);
    
    // 修改不会影响任何文件
    printf("尝试同步到文件会失败: ");
    if (msync(ptr, size, MS_SYNC) == -1) {
        perror("msync failed");
    }
    
    munmap(ptr, size);
}

int main() {
    printf("=== 内存映射对比实验 ===\n");
    file_mapping_demo();
    anonymous_mapping_demo();
    return 0;
}
