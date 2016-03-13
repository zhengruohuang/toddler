#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/kernel.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


static struct hal_exports *hexp;
static void asmlinkage (*kernel_entry)(struct hal_exports *exp);


void init_kernel()
{
    kprintf("Initializing kernel\n");
    
    /*
     * Build HAL exports
     */
    hexp = (struct hal_exports *)kalloc(sizeof(struct hal_exports));
    
    // Kernel exprts
    hexp->kernel = (struct kernel_exports *)kalloc(sizeof(struct kernel_exports));
    
    // General functions
    hexp->kprintf = kprintf;
    hexp->halt = wrap_halt;
    
    // Topology
    hexp->num_cpus = num_cpus;
    
    // Physical memory info
    hexp->free_mem_start_addr = get_bootparam()->free_pfn_start;
    hexp->paddr_space_end = paddr_space_end;
    hexp->get_next_mem_zone = get_next_mem_zone;
    
    // Mapping
    hexp->kernel_map = wrap_kernel_map;
    hexp->user_map = wrap_user_map;
    
    
    /*
     * Call kernel's entry
     */
    kprintf("\tKernel entry: %p\n", get_bootparam()->kernel_entry_addr);
    kernel_entry = (void *)get_bootparam()->kernel_entry_addr;
    kernel_entry(hexp);
    
    kprintf("Kernel has been initialized!\n");
}
