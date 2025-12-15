#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char path[256];
    FILE *maps;
    char line[512];
    
    // 获取可执行文件路径
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        printf("Cannot read executable path\n");
        return 1;
    }
    path[len] = '\0';
    
    printf("Executable path: %s\n", path);
    
    // 读取内存映射
    maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        printf("Cannot open /proc/self/maps\n");
        return 1;
    }
    
    // 查找可执行文件的代码段
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, path) && strstr(line, "r-xp")) {
            unsigned long base_addr;
            if (sscanf(line, "%lx", &base_addr) == 1) {
                printf("Code segment base address: 0x%lx\n", base_addr);
                printf("Full line: %s", line);
                fclose(maps);
                
                // 测试addr2line
                char cmd[512];
                snprintf(cmd, sizeof(cmd), "addr2line -fi -e %s 0x%lx", path, 0x2509);
                printf("Testing addr2line: %s\n", cmd);
                system(cmd);
                
                return 0;
            }
        }
    }
    
    fclose(maps);
    printf("Code segment not found\n");
    return 1;
}
