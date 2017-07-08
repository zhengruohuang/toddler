#ifndef __ARCH_PPC32_COMMON_INCLUDE_BOOTPARAM__
#define __ARCH_PPC32_COMMON_INCLUDE_BOOTPARAM__


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
    int boot_dev;
    int boot_dev_info;
    
    // AP starter
    ulong ap_entry_addr;
    ulong ap_page_table_ptr;
    ulong ap_stack_top_ptr;
    
    // Core image
    ulong coreimg_load_addr;
    
    // HAL
    int hal_start_flag;
    ulong hal_entry_addr;
    ulong hal_vaddr_end;
    ulong hal_vspace_end;
    ulong bsp_area_addr;
    
    // Kernel
    ulong kernel_entry_addr;
    
    // Video info
    int video_mode;
    union {
        struct {
            ulong fb_addr;
            int fb_res_x, fb_res_y;
            int fb_bits_per_pixel;
            int fb_bytes_per_line;
        };
        
        struct {
            ulong serial_addr;
        };
    };
    
    // Pageing
    ulong pht_addr;
    ulong pde_addr;
    ulong pte_addr;
    
    // Address where free memory starts
    ulong free_addr_start;
    ulong free_pfn_start;
        
    // Memory size
    u64 mem_size;
    
    // Memory zones
    int mem_zone_count;
    struct boot_mem_zone mem_zones[32];
} packedstruct;


#endif
