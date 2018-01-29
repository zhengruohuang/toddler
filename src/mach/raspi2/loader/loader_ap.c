#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"
#include "common/include/page.h"
#include "common/include/reg.h"
#include "loader/include/print.h"
#include "loader/include/lib.h"


static void stop(ulong val)
{
    if (!val) {
        val = 0xbeef;
    }
    
    while (1) {
        __asm__ __volatile__
        (
            "mov r0, %[reg];"
            :
            : [reg] "r" (val)
        );
    }
}


/*
 * Boot parameters
 */
extern struct boot_parameters boot_param;


/*
 * Paging
 */
static void setup_paging()
{
    ulong page_table = boot_param.ap_page_dir;
    
    lprintf("Setup paging @ %lx ... ", page_table);
    
    // Copy the page table address to cp15
    write_trans_tab_base0(page_table);
    
    // Enable permission check for domain0
    struct domain_access_ctrl_reg domain;
    read_domain_access_ctrl(domain.value);
    domain.domain0 = 0x1;
    write_domain_access_ctrl(domain.value);

    // Enable the MMU
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.mmu_enabled = 1;
    write_sys_ctrl(sys_ctrl.value);
    
    // Caches
    lprintf("Dcache: %d\n", sys_ctrl.dcache_enabled);
    
    lprintf("Done!\n");
}

/*
 * Jump to HAL!
 */
static void (*hal_entry)();

static void jump_to_hal()
{
    lprintf("Jump to HAL @ %p\n", boot_param.hal_entry_addr);
    
    __asm__ __volatile__ (
        // Move to System mode
        "cpsid aif, #0x1f;"
        
        // Set up stack top
        "mov sp, %[stack];"
        
        // Pass the argument
        "mov r0, %[bp];"
        
        // Call C
        "mov pc, %[hal];"
        :
        : [stack] "r" (boot_param.ap_stack_top), [bp] "r" (&boot_param), [hal] "r" (boot_param.hal_entry_addr)
        : "sp", "r0", "memory"
    );
}


/*
 * Loader entry
 */
void loader_ap_entry()
{
    lprintf("AP started!\n");
//     while (1);
    
    setup_paging();
    
    jump_to_hal();
    
    while (1);
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
