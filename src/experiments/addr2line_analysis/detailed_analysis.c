#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== DETAILED ADDR2LINE ANALYSIS ===\n\n");
    
    printf("KEY FINDINGS FROM PREVIOUS EXPERIMENT:\n");
    printf("=====================================\n\n");
    
    printf("1. SYMBOL TABLE ADDRESSES:\n");
    printf("   main:      0x2509 (from objdump -t)\n");
    printf("   toy_malloc: 0x3534 (from objdump -t)\n\n");
    
    printf("2. SECTION INFORMATION:\n");
    printf("   .text section: Address=0x2420, Offset=0x2420 (from readelf -S)\n\n");
    
    printf("3. LOAD SEGMENT INFORMATION:\n");
    printf("   LOAD segment 1: File Offset=0x0000, Virt Addr=0x0000\n");
    printf("   LOAD segment 2: File Offset=0x2000, Virt Addr=0x2000 ← CODE SEGMENT!\n");
    printf("   LOAD segment 3: File Offset=0x4000, Virt Addr=0x4000\n\n");
    
    printf("4. RUNTIME MAPPING:\n");
    printf("   r-xp segment: File Offset=0x1000, Virt Addr=0x555879619000\n\n");
    
    printf("5. ADDR2LINE TEST RESULTS:\n");
    printf("   Symbol table (0x2509): WORKS ✓\n");
    printf("   Symbol table (0x3534): WORKS ✓\n");
    printf("   Our formula (0x25f2):  WORKS ✓\n");
    printf("   Our formula (0x36d7):  WORKS ✓\n\n");
    
    printf("6. CRITICAL INSIGHT:\n");
    printf("====================\n\n");
    
    printf("ADDR2LINE ACCEPTS: RELATIVE TO LOAD SEGMENT!\n\n");
    
    printf("EVIDENCE:\n");
    printf("- Symbol addresses (0x2509, 0x3534) work with addr2line\n");
    printf("- These are relative to LOAD segment 2 (File Offset=0x2000)\n");
    printf("- NOT relative to .text section (which starts at 0x2420)\n");
    printf("- NOT relative to ELF file start (which would be 0x4509, 0x5534)\n\n");
    
    printf("PROOF:\n");
    printf("- LOAD segment 2: File Offset=0x2000, Virt Offset=0x2000\n");
    printf("- Symbol addresses are already relative to this LOAD segment\n");
    printf("- That's why they work directly with addr2line\n\n");
    
    printf("OUR FORMULA:\n");
    printf("file_relative_offset = virtual_addr - load_base + file_offset\n\n");
    
    printf("BUT WAIT! THIS FORMULA IS WRONG!\n");
    printf("The correct formula should be:\n");
    printf("addr2line_offset = virtual_addr - load_base\n\n");
    
    printf("WHY OUR FORMULA \"WORKS\":\n");
    printf("- In our case: load_base_virt = 0x563810da2000\n");
    printf("- In our case: file_offset = 0x2000\n");
    printf("- Symbol addresses are already: relative_to_load_segment = 0x2509\n");
    printf("- Our formula gives: 0x25f2 = 0x5f2 + 0x2000\n");
    printf("- This happens to work because of specific memory layout!\n\n");
    
    printf("CONCLUSION:\n");
    printf("==========\n");
    printf("Addr2line accepts addresses RELATIVE TO LOAD SEGMENT!\n");
    printf("Symbol table addresses are already in this format.\n");
    printf("Our formula is coincidentally correct for this specific case.\n");
    
    return 0;
}
