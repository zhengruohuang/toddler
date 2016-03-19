#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/kernel.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


static struct hal_exports *hexp;
static void asmlinkage (*kernel_entry)(struct hal_exports *exp);

struct kernel_exports *kernel;


void init_kernel()
{
    kprintf("Initializing kernel\n");
    
    /*
     * Build HAL exports
     */
    hexp = (struct hal_exports *)kalloc(sizeof(struct hal_exports));
    
    // Kernel exprts
    kernel = (struct kernel_exports *)kalloc(sizeof(struct kernel_exports));
    hexp->kernel = kernel;
    
    // General functions
    hexp->kprintf = kprintf;
    hexp->halt = wrap_halt;
    
    // Kernel info
    hexp->kernel_page_dir_pfn = KERNEL_PDE_PFN;
    
    // Topology
    hexp->num_cpus = num_cpus;
    
    // Physical memory info
    hexp->free_mem_start_addr = PFN_TO_ADDR(get_bootparam()->free_pfn_start);
    hexp->paddr_space_end = paddr_space_end;
    hexp->get_next_mem_zone = get_next_mem_zone;
    
    // Mapping
    hexp->user_map = wrap_user_map;
    
    // AS
    hexp->init_context = init_thread_context;
    hexp->sleep = wrap_halt;
    hexp->switch_context = switch_context;
    
    /*
     * Call kernel's entry
     */
    kprintf("\tKernel entry: %p\n", get_bootparam()->kernel_entry_addr);
    kernel_entry = (void *)get_bootparam()->kernel_entry_addr;
    kernel_entry(hexp);
    
    kprintf("Kernel has been initialized!\n");
}
