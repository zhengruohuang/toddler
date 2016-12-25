#include "loader/periph.h"
#include "common/include/bootparam.h"


/*
 * Helper functions
 */
static void panic()
{
    lprintf("Panic!");
    
    while (1) {
        // do nothing
    }
}


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
 * Boot parameters
 */
struct boot_parameters boot_param;

static void build_bootparam()
{
    lprintf("Building boot parameters @ %x ... ", (u32)&boot_param);
    
    // Boot info
    boot_param.boot_dev = 0;
    boot_param.boot_dev_info = 0;
    
    // Loader func
    boot_param.loader_func_type_ptr = 0;
    
    // AP starter
    boot_param.ap_entry_addr = 0;
    boot_param.ap_page_table_ptr = 0;
    boot_param.ap_stack_top_ptr = 0;
    
    // HAL & kernel
    boot_param.hal_start_flag = 0;
    boot_param.hal_entry_addr = 0;
    boot_param.hal_vaddr_end = 0;
    boot_param.hal_vspace_end = 0;
    boot_param.kernel_entry_addr = 0;
    
    // Memory map
    boot_param.free_addr_start = 2 * 1024 * 1024;
    boot_param.mem_size = (u64)0x40000000ull;
    
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
    
    // Usable memory (2MB to 256MB)
    boot_param.mem_zones[zone_count].start_paddr = 0x200000;
    boot_param.mem_zones[zone_count].len = PHYS_MEM_SIZE - 0x200000;
    boot_param.mem_zones[zone_count].type = 1;
    zone_count++;
    
    boot_param.mem_zone_count = zone_count;
    
    lprintf("Done!\n");
}


/*
 * Paging
 */
struct tlb_entry {
    u32 test;
} packetstruct;

static void setup_paging()
{
    lprintf("Setup TLB entry for HAL and kernel @ highest 2MB, TLB entry @ %x ... ", (u32)0);
    
    // Map highest 2MB to physical 0 to 2MB
    // Note that in MIPS each one TLB entries maps 2 consecutive virtual pages to 2 different
    
    lprintf("Done!\n");
}


/*
 * Main
 */
void main()
{
    init_bss();
    init_periph();
    
    lprintf("Welcome to Toddler!\n");
    
    build_bootparam();
    setup_paging();
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
