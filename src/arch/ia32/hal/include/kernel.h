#ifndef __ARCH_IA32_HAL_INCLUDE_KERNEL__
#define __ARCH_IA32_HAL_INCLUDE_KERNEL__


#include "common/include/data.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


/*
 * Wrappers
 */
extern int asmlinkage wrap_user_map(ulong page_dir, ulong vaddr, ulong paddr, size_t size, int exec, int write, int cacheable, int override);
extern ulong asmlinkage wrap_get_paddr(ulong page_dir_pfn, ulong vaddr);
extern int asmlinkage wrap_load_exe(ulong image_start, ulong dest_page_dir_pfn,
                                    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out);
extern void asmlinkage wrap_init_addr_space(ulong page_dir_pfn);
extern void asmlinkage wrap_halt();
extern void asmlinkage wrap_sleep();
extern int asmlinkage wrap_get_cur_cpu_id();


/*
 * Mem zone
 */
extern ulong paddr_space_end;

extern int asmlinkage get_next_mem_zone(struct kernel_mem_zone *cur);
extern void init_kmem_zone();
extern void full_direct_map();

/*
 * Init kernel
 */
extern struct kernel_exports *kernel;

extern void init_kernel();


#endif
