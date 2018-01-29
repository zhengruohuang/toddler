#ifndef __ARCH_ARMV7_HAL_INCLUDE_MEM__
#define __ARCH_ARMV7_HAL_INCLUDE_MEM__


#include "common/include/data.h"
#include "hal/include/kalloc.h"


/*
 * Page allocator
 */
extern ulong palloc(int count);


/*
 * Mapping
 */
extern void init_user_page_dir(ulong page_dir_pfn);

extern ulong get_paddr(ulong page_dir_pfn, ulong vaddr);

extern int user_indirect_map_array(
    ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length,
    int exec, int write, int cacheable, int overwrite);
extern int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length);

extern void kernel_map_per_cpu_area(ulong vstart, ulong pstart, ulong size);

extern void init_map();


/*
 * TLB
 */
extern void invalidate_tlb_array(ulong asid, ulong vaddr, size_t size);
extern void init_tlb();


#endif
