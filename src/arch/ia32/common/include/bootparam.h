#ifndef __ARCH_IA32_COMMON_INCLUDE_BOOTPARAM__
#define __ARCH_IA32_COMMON_INCLUDE_BOOTPARAM__


#include "common/include/data.h"


struct boot_mem_zone {
    u64 start_paddr;
    u64 len;
    u32 type;
} packedstruct;

struct boot_param {
    // Boot device
    u32 boot_dev;
    u32 boot_dev_info;
    
    // Addresses for loader
    u32 what_to_load_addr;
    u32 ap_entry_addr;
    u32 ap_page_dir_pfn_addr;
    
    // Video info
    u8 video_mode;
    
    // HAL start up flags
    u32 hal_start_flag;
    u32 cursor_row;
    u32 cursor_col;
    
    // Memory size
    u64 mem_size;
    
    // Memory zones
    u32 mem_zone_count;
    struct boot_mem_zone mem_zones[32];
} packedstruct;


#endif
