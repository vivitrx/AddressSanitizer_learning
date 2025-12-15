#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void analyze_maps();
int main() { analyze_maps(); return 0; }

void analyze_maps() {
    FILE *maps;
    char line[512];
    char executable_path[256];
    
    // 获取可执行文件路径
    ssize_t len = readlink("/proc/self/exe", executable_path, sizeof(executable_path) - 1);
    if (len == -1) {
        printf("Cannot get executable path\n");
        return;
    }
    executable_path[len] = '\0';
    
    printf("Executable path: %s\n", executable_path);
    printf("Main function address: %p\n", main);
    printf("\n=== /proc/self/maps analysis ===\n");
    
    // 读取内存映射
    maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        printf("Cannot open /proc/self/maps\n");
        return;
    }
    
    printf("Memory mappings containing '%s':\n", executable_path);
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, executable_path)) {
            printf("%s", line);
        }
    }
    
    fclose(maps);
    
    printf("\n=== Looking for r-xp (executable) segment ===\n");
    maps = fopen("/proc/self/maps", "r");
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, executable_path) && strstr(line, "r-xp")) {
            printf("Found executable segment: %s", line);
            
            // 解析地址和偏移
            unsigned long addr_start, addr_end, offset;
            if (sscanf(line, "%lx-%lx %*4s %lx", &addr_start, &addr_end, &offset) == 3) {
                printf("  Virtual address range: 0x%lx - 0x%lx\n", addr_start, addr_end);
                printf("  Load base (addr_start): 0x%lx\n", addr_start);
                printf("  File offset: 0x%lx\n", offset);
                printf("  Main function offset: 0x%lx\n", (unsigned long)main - addr_start);
                
                // 计算addr2line需要的偏移
                unsigned long addr2line_offset = (unsigned long)main - addr_start + offset;
                printf("  Addr2line offset: 0x%lx\n", addr2line_offset);
            }
            break;
        }
    }
    fclose(maps);
}
