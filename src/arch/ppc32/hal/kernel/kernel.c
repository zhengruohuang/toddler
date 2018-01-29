#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/kexport.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/time.h"
#include "hal/include/kernel.h"


static struct hal_exports *hexp;
static void asmlinkage (*kernel_entry)(struct hal_exports *exp);

struct kernel_exports *kernel;


/*
 * Wrappers
 */
static ulong wrap_io_in(ulong port, ulong size)
{
    return 0;
}

static void wrap_io_out(ulong port, ulong size, ulong data)
{
}


/*
 * Init kernel
 */
void init_kernel()
{
    struct boot_parameters *boot_param = get_bootparam();
    
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
    hexp->time = get_system_time;
    hexp->halt = halt;
    
    // Kernel info
    hexp->kernel_page_dir_pfn = ADDR_TO_PFN(boot_param->pde_addr);
    
    // Core image info
    hexp->coreimg_load_addr = boot_param->coreimg_load_addr;
    
    // MP
    hexp->num_cpus = num_cpus;
    hexp->get_cur_cpu_id = get_cpu_id;
    
    // Physical memory info
    hexp->free_mem_start_addr = PFN_TO_ADDR(boot_param->free_pfn_start);
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
    hexp->map_user = user_indirect_map_array;
    hexp->unmap_user = user_indirect_unmap_array;
    hexp->get_paddr = get_paddr;
    
    // Address space
    hexp->user_page_dir_page_count = 1;
    hexp->vaddr_space_end = USER_VADDR_SPACE_END;
    hexp->init_addr_space = init_user_page_dir;
    
    // Context
    hexp->init_context = init_thread_context;
    hexp->set_context_param = set_thread_context_param;
    hexp->switch_context = switch_context;
    hexp->set_syscall_return = set_syscall_return;
    
    // TLB
    hexp->invalidate_tlb = invalidate_tlb_array;
    
    /*
     * Call kernel's entry
     */
    kernel_entry = (void *)boot_param->kernel_entry_addr;
    kprintf("\tKernel entry: %p\n", kernel_entry);
    kernel_entry(hexp);
    
    kprintf("Kernel has been initialized!\n");
}
