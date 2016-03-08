#ifndef __ARCH_IA32_COMMON_INCLUDE_BOOTPARAM__
#define __ARCH_IA32_COMMON_INCLUDE_BOOTPARAM__


#include "common/include/data.h"


struct boot_mem_zone {
    u64 start_paddr;
    u64 len;
    u32 type;
} packedstruct;

struct boot_parameters {
    // Boot device
    u32 boot_dev;
    u32 boot_dev_info;
    
    // Pointers
    ulong loader_func_type_ptr;
    ulong ap_entry_addr_ptr;
    ulong bios_invoker_addr_ptr;
    
    // HAL
    u32 hal_start_flag;
    u32 hal_entry_addr;
    u32 hal_vaddr_end;
    u32 hal_vspace_end;
    
    // Kernel
    u32 kernel_entry_addr;
    
    // Video info
    u32 video_mode;
    u32 framebuffer_addr;
    u32 res_x;
    u32 res_y;
    u32 bits_per_pixel;
    u32 bytes_per_line;
    
    // Cursor
    u32 cursor_row;
    u32 cursor_col;
    
    // EBDA
    u32 ebda_addr;
    
    // First free PFN
    u32 free_pfn_start;
        
    // Memory size
    u64 mem_size;
    
    // Memory zones
    u32 mem_zone_count;
    struct boot_mem_zone mem_zones[32];
} packedstruct;


#endif
