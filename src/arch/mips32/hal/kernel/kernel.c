#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/int.h"
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
    hexp->time = NULL;
    hexp->halt = wrap_halt;
    
    // Kernel info
    hexp->kernel_page_dir_pfn = 0;
    
    // Core image info
    hexp->coreimg_load_addr = 0;
    
    // MP
    hexp->num_cpus = num_cpus;
    hexp->get_cur_cpu_id = wrap_get_cur_cpu_id;
    
    // Physical memory info
    hexp->free_mem_start_addr = PFN_TO_ADDR(get_bootparam()->free_pfn_start);
    hexp->paddr_space_end = paddr_space_end;
    hexp->get_next_mem_zone = get_next_mem_zone;
    
    // IO Ports
    hexp->io_in = wrap_io_in;
    hexp->io_out = wrap_io_out;
    
    // Interrupt
    hexp->disable_local_interrupt = disable_local_int;
    hexp->enable_local_interrupt = enable_local_int;
    hexp->restore_local_interrupt = restore_local_int;
    
    // Mapping
    hexp->map_user = wrap_user_map;
    hexp->unmap_user = wrap_user_unmap;
    hexp->get_paddr = wrap_get_paddr;
    
    // Load image
    hexp->load_exe = wrap_load_exe;
    
    // AS
    hexp->init_addr_space = wrap_init_addr_space;
    hexp->init_context = init_thread_context;
    hexp->switch_context = switch_context;
    hexp->set_syscall_return = set_syscall_return;
    hexp->sleep = wrap_sleep;
    hexp->yield = wrap_yield;
    hexp->ksyscall = wrap_ksyscall;
    
    // TLB
    hexp->invalidate_tlb = wrap_invalidate_tlb;
    
    /*
     * Call kernel's entry
     */
    kernel_entry = (void *)get_bootparam()->kernel_entry_addr;
    kprintf("\tKernel entry: %p\n", kernel_entry);
    kernel_entry(hexp);
    
    kprintf("Kernel has been initialized!\n");
}
