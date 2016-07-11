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
extern int wrap_user_map(ulong page_dir, ulong vaddr, ulong paddr, size_t size, int exec, int write, int cacheable, int override);
extern int wrap_user_unmap(ulong page_dir_pfn, ulong vaddr, ulong paddr, ulong size);

extern ulong wrap_get_paddr(ulong page_dir_pfn, ulong vaddr);
extern int wrap_load_exe(ulong image_start, ulong dest_page_dir_pfn,
                                    ulong *entry_out, ulong *vaddr_start_out, ulong *vaddr_end_out);
extern void wrap_init_addr_space(ulong page_dir_pfn);
extern int wrap_get_cur_cpu_id();

extern ulong wrap_io_in(ulong port, ulong size);
extern void wrap_io_out(ulong port, ulong size, ulong data);

extern void wrap_invalidate_tlb(ulong asid, ulong vaddr, size_t size);

extern void wrap_halt();
extern void wrap_sleep();
extern void wrap_yield();


/*
 * Mem zone
 */
extern ulong paddr_space_end;

extern int get_next_mem_zone(struct kernel_mem_zone *cur);
extern void init_kmem_zone();
extern void full_direct_map();

/*
 * Init kernel
 */
extern struct kernel_exports *kernel;

extern void init_kernel();
extern void kernel_dispatch(struct kernel_dispatch_info *kdi);


#endif
