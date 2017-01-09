#ifndef __KERNEL_INCLUDE_HAL__
#define __KERNEL_INCLUDE_HAL__


#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/task.h"
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
    int (*load_exe)(ulong image_start, ulong dest_page_dir_pfn,
                               ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out);
    
    // Task
    void (*init_addr_space)(ulong page_dir_pfn);
    void (*init_context)(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode);
    void (*set_context_param)(struct context *context, ulong param);
    void (*switch_context)(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid,
                                      struct thread_control_block *tcb);
    void (*set_syscall_return)(struct context *context, int succeed, ulong return0, ulong return1);
    void (*sleep)();
    void (*yield)();
    int (*ksyscall)(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2);
    
    // TLB
    void (*invalidate_tlb)(ulong asid, ulong vaddr, size_t size);
};


/*
 * Paging
 */
#ifndef PAGE_BITS
#define PAGE_BIST   12  // FIXME: this should be moved to hal
#endif

#ifndef PFN_BITS
#define PFN_BITS    sizeof(unsigned long) * 8 - PAGE_BITS
#endif


/*
 * Externs and wrappers only for kernel
 */
#ifndef __HAL__
extern struct hal_exports *hal;
#include "kernel/include/sync.h"
extern spinlock_t kprintf_lock;
#endif

/*
 * Frequent function wrappers
 */
#ifndef __HAL__
#define kprintf(...)                \
do {                                \
    spin_lock_int(&kprintf_lock);   \
    hal->kprintf(__VA_ARGS__);      \
    spin_unlock_int(&kprintf_lock); \
} while (0)

#define halt            (hal->halt)

#define kernel_unreachable() do {   \
    do {                            \
        hal->yield();               \
    } while (1);                    \
    kprintf("kputs.c: Should never reach here!\n"); \
    do {                            \
        hal->sleep();               \
    } while (1);                    \
} while (0)
#endif


/*
 * Debugging macros
 */
#ifndef disp_src_info
#define disp_src_info(f, b, l) kprintf("[SRC] File: %s, Base: %s, Line: %d\n", f, b, l)
#endif

#ifndef assert
#define assert(exp)                         \
    do {                                    \
        if (exp) {                          \
        } else {                            \
            kprintf("[ASSERT] ");           \
            kprintf(#exp);                  \
            kprintf("\n");                  \
            disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
            halt();                         \
        }                                   \
    } while (0)
#endif

#ifndef warn
#define warn(fmt, ...)                  \
    do {                                \
        kprintf("[WARN] ");             \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
    } while (0)
#endif

#ifndef panic
#define panic(fmt, ...)                 \
    do {                                \
        kprintf("[PANIC] ");            \
        kprintf(fmt, ##__VA_ARGS__);    \
        kprintf("\n");                  \
        disp_src_info(__FILE__, __BASE_FILE__, __LINE__);   \
        halt();                                             \
    } while (0)
#endif


#endif
