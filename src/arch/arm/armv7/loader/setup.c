#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/reg.h"


void setup_paging(ulong page_dir, ulong dram_start_paddr, ulong dram_end_paddr,
                  ulong hal_area_base_paddr)
{
    int i;
    volatile struct l1table *page_table = (volatile struct l1table *)page_dir;
    
    // Set up an 1 to 1 mapping for all 4GB, rw for everyone
    // Note that each entry maps 1MB, and there are 4K entries -> 16KB L1 page table
    for (i = 0; i < 4096; i++) {
        page_table->value_l1section[i].value = 0;
        page_table->value_l1section[i].present = 1;
        page_table->value_l1section[i].user_write = 1;      // AP[1:0] = 01
        page_table->value_l1section[i].user_access = 0;     // Kernel RW, user no access
        page_table->value_l1section[i].sfn = i;
    }
    
    // Enable cache for DRAM
    for (i = dram_start_paddr >> 20; i < (dram_end_paddr >> 20); i++) {
        // TEX[2:0]:C:B = 000:1:1
        // Cacheable, write-back, write-allocate
        page_table->value_l1section[i].cache_inner = 0x3;
    }
    
    // Map the highest 1MB (HAL + kernel) to physical 1MB to 2MB
    page_table->value_l1section[4095].cache_inner = 0x3;
    page_table->value_l1section[4095].sfn = hal_area_base_paddr >> 20;
}

void enable_mmu(ulong page_dir)
{
    // Copy the page table address to cp15
    write_trans_tab_base0(page_dir);
    
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
}

void enable_caches()
{
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.dcache_enabled = 1;
    sys_ctrl.icache_enabled = 1;
    write_sys_ctrl(sys_ctrl.value);
}

void enable_bpred()
{
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.bpred_enabled = 1;
    write_sys_ctrl(sys_ctrl.value);
}

void call_hal_entry(ulong entry, ulong stack, ulong bp)
{
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
        : [stack] "r" (stack), [bp] "r" (bp), [hal] "r" (entry)
        : "sp", "r0", "memory"
    );
}
