#ifndef __COMMON_INCLUDE_KEXPORT__
#define __COMMON_INCLUDE_KEXPORT__


#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/proc.h"


/*
 * Kernel exported variables and functions
 */
struct kernel_exports {
    ulong (*palloc_tag)(int count, int tag);
    ulong (*palloc)(int count);
    int (*pfree)(ulong pfn);
    void (*dispatch)(ulong sched_id, struct kernel_dispatch_info *int_info);
};


/*
 * Structures needed by HAL exports
 */
struct kernel_mem_zone {
    ulong start;
    ulong len;
    int usable;
    int mapped;
    int tag;
    int inuse;
    int kernel;
    int swappable;
};


/*
 * HAL exported variables and functions
 */
struct hal_exports {
    // Kernel exports, this field is set by kernel
    struct kernel_exports *kernel;
    
    // General functions
    int asmlinkage (*kprintf)(char *s, ...);
    void (*time)(ulong *high, ulong *low);
    void (*halt)();
    
    // Kernel info
    ulong kernel_page_dir_pfn;
    
    // Core image
    ulong coreimg_load_addr;
    
    // MP
    int num_cpus;
    int (*get_cur_cpu_id)();
    
    // Physical memory info
    ulong free_mem_start_addr;
    ulong paddr_space_end;
    int (*get_next_mem_zone)(struct kernel_mem_zone *cur);
    
    // IO Ports
    ulong (*io_in)(ulong addr, ulong size);
    void (*io_out)(ulong addr, ulong size, ulong data);
    
    // Interrupts
    int (*disable_local_interrupt)();
    void (*enable_local_interrupt)();
    void (*restore_local_interrupt)(int enabled);
    
    // Mapping
    int (*map_user)(ulong page_dir_pfn, ulong vaddr, ulong paddr,
                               size_t length, int exec, int write, int cacheable, int overwrite);
    int (*unmap_user)(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length);
    ulong (*get_paddr)(ulong page_dir_pfn, ulong vaddr);
    
    // Load image
    //int (*load_exe)(ulong image_start, ulong dest_page_dir_pfn,
    //                           ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out);
    
    // Address space
    int user_page_dir_page_count;
    ulong vaddr_space_end;
    void (*init_addr_space)(ulong page_dir_pfn);
    
    // Context
    void (*init_context)(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode);
    void (*set_context_param)(struct context *context, ulong param);
    void (*switch_context)(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb);
    void (*set_syscall_return)(struct context *context, int succeed, ulong return0, ulong return1);
    //void (*sleep)();
    //void (*yield)();
    
    // TLB
    void (*invalidate_tlb)(ulong asid, ulong vaddr, size_t size);
};


#endif
