#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "loader/ofw.h"


/*
 * BSS
 */
extern int __bss_start;
extern int __bss_end;

static void init_bss()
{
    int *cur;
    
    for (cur = &__bss_start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}


/*
 * MMU and caches
 */
static void init_mmu()
{
}

static void init_caches()
{
}


/*
 * Boot parameters
 */
struct boot_parameters boot_param;

static void build_bootparam()
{
    //lprintf("Building boot parameters @ %x ... ", (u32)&boot_param);
    
    // Boot info
    boot_param.boot_dev = 0;
    boot_param.boot_dev_info = 0;
    
    // Loader func
    boot_param.loader_func_type_ptr = 0;
    
    // AP starter
    boot_param.ap_entry_addr = 0;
    boot_param.ap_page_table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
    // Core image
    //boot_param.coreimg_load_addr = coreimg_start_addr;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = 0x80180000;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.free_addr_start = 2 * 1024 * 1024;
    //boot_param.free_pfn_start = ADDR_TO_PFN(boot_param.free_addr_start);
    //boot_param.mem_size = (u64)memory_size;
    
    // Memory zones
    u32 zone_count = 0;
    
    // Reserved by HW and loader programs (lowest 64KB)
    boot_param.mem_zones[zone_count].start_paddr = 0x0;
    boot_param.mem_zones[zone_count].len = 0x8000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Core image (64KB to 1MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x8000;
    boot_param.mem_zones[zone_count].len = 0x100000 - 0x8000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // HAL and kernel (1MB to 2MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x100000;
    boot_param.mem_zones[zone_count].len = 0x100000;
    boot_param.mem_zones[zone_count].type = 0;
    zone_count++;
    
    // Usable memory (2MB to 128MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x200000;
    //boot_param.mem_zones[zone_count].len = memory_size - 0x200000;
    boot_param.mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    //lprintf("Done!\n");
}


/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{

}


/*
 * Entry point
 */
void loader_entry(ulong ofw_entry)
{
    init_bss();
    ofw_init(ofw_entry);
    ofw_print_mem_zones();
    ofw_test_translation();
    ofw_setup_displays();
    ofw_tree_build();
    
    init_mmu();
    init_caches();
    
    jump_to_hal();
    
    while (1) {
        //panic();
    }
}
