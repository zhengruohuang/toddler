#ifndef __ARCH_MIPS32_COMMON_INCLUDE_BOOTPARAM__
#define __ARCH_MIPS32_COMMON_INCLUDE_BOOTPARAM__


#include "common/include/data.h"

#define VIDEO_NONE          0
#define VIDEO_FRAMEBUFFER   1
#define VIDEO_TEXT          2
#define VIDEO_UART          3

struct boot_mem_zone {
    u64 start_paddr;
    u64 len;
    u32 type;
} packedstruct;

struct boot_parameters {
    // Boot device
    u32 boot_dev;
    u32 boot_dev_info;
    
    // Loader
    ulong loader_func_type_ptr;
    
    // AP starter
    ulong ap_entry_addr;
    ulong ap_page_table_ptr;
    ulong ap_stack_top_ptr;
    
    // HAL
    u32 hal_start_flag;
    ulong hal_entry_addr;
    ulong hal_vaddr_end;
    ulong hal_vspace_end;
    
    // Kernel
    ulong kernel_entry_addr;
    
    // Video info
    u32 video_mode;
    u32 cursor_row;
    u32 cursor_col;
    ulong framebuffer_addr;
    u32 res_x;
    u32 res_y;
    u32 bytes_per_pixel;
    u32 bytes_per_line;
    
    // Address where free memory starts
    ulong free_addr_start;
        
    // Memory size
    u64 mem_size;
    
    // Memory zones
    u32 mem_zone_count;
    struct boot_mem_zone mem_zones[32];
} packedstruct;


#endif
