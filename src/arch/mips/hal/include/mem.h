#ifndef __ARCH_MIPS32_HAL_INCLUDE_MEM__
#define __ARCH_MIPS32_HAL_INCLUDE_MEM__


#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/reg.h"
#include "common/include/page.h"
#include "hal/include/percpu.h"
#include "hal/include/cpu.h"


/*
 * Map
 */
extern void init_user_page_dir(ulong page_dir_pfn);

extern ulong get_paddr(ulong page_dir_pfn, ulong vaddr);

extern int user_indirect_map_array(
    ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length,
    int exec, int write, int cacheable, int overwrite);
extern int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length);


/*
 * TLB
 */
struct tlb_entry {
    struct cp0_entry_hi hi;
    struct cp0_page_mask pm;
    
    struct cp0_entry_lo lo0;
    struct cp0_entry_lo lo1;
} packedstruct;

ext_per_cpu(struct page_frame *, cur_page_dir);

extern int tlb_refill_kernel(ulong addr);
extern int tlb_refill_user(ulong addr);

extern void invalidate_tlb_array(ulong asid, ulong vaddr, size_t size);

extern void init_tlb();


/*
 * KAlloc
 */
extern ulong palloc(int count);
extern void kfree(void *ptr);
extern void *kalloc(size_t size);
extern void init_kalloc();


#endif
