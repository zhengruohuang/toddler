#ifndef __ARCH_PPC32_HAL_INCLUDE_MEM__
#define __ARCH_PPC32_HAL_INCLUDE_MEM__


#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/percpu.h"


/*
 * PHT
 */
#define KERNEL_PHT_PERSIST      1
#define KERNEL_PHT_REGULAR      0
#define KERNEL_PHT_TEMPORARY    -1

ext_per_cpu(ulong, cur_page_dir_pfn);
ext_per_cpu(struct page_frame *, cur_page_dir);

extern void fill_pht_by_page(ulong asid, ulong vaddr, ulong ppfn, ulong count, int io, int priority);
extern void fill_pht_by_addr(ulong asid, ulong vaddr, ulong paddr, ulong size, int io, int priority);
extern ulong translate_pht(ulong asid, ulong vaddr);
extern int evict_pht(ulong asid, ulong vaddr);

extern void fill_kernel_pht(ulong vstart, ulong len, int io, int priority);

extern void init_pht();


/*
 * KAlloc
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


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


#endif
