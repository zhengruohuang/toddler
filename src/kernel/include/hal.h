#ifndef __KERNEL_INCLUDE_HAL__
#define __KERNEL_INCLUDE_HAL__


#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/task.h"


/*
 * Kernel exported variables and functions
 */
struct kernel_exports {
    ulong asmlinkage (*alloc_page)(int count, int tag);
    void asmlinkage (*dispatch)(ulong sched_id, struct kernel_dispatch_info *int_info);
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
    void asmlinkage (*halt)();
    
    // Kernel info
    ulong kernel_page_dir_pfn;
    
    // Topology
    int num_cpus;
    
    // MP
    int asmlinkage (*get_cur_cpu_id)();
    
    // Physical memory info
    ulong free_mem_start_addr;
    ulong paddr_space_end;
    int asmlinkage (*get_next_mem_zone)(struct kernel_mem_zone *cur);
    
    // Mapping
    int asmlinkage (*user_map)(ulong page_dir, ulong vaddr, ulong paddr, size_t size, int exec, int write, int cacheable);
    
    // Addr space
    void asmlinkage (*init_context)(struct context *context, ulong entry, ulong stack_top, int user_mode);
    
    // Task
    void asmlinkage (*sleep)();
    void asmlinkage (*switch_context)(ulong sched_id, struct context *context, ulong page_dir_pfn, int user_mode, ulong asid);
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
#endif

/*
 * Frequent function wrappers
 */
#ifndef __HAL__
#define kprintf         (hal->kprintf)
#define halt            (hal->halt)
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
