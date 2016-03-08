#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"


void kernel_indirect_map(ulong vaddr, ulong paddr, int disable_cache, int override)
{
    // PDE
    struct page_frame *page = (struct page_frame *)KERNEL_PDE_PADDR;
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        page->value_pde[index].pfn = palloc(1);
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (page->value_u32[index] && !override) {
        assert(
            page->value_pte[index].pfn == ADDR_TO_PFN(paddr) &&
            page->value_pde[index].present &&
            page->value_pde[index].rw
        );
        
        if (disable_cache) {
            page->value_pde[index].cache_disabled = 1;
        }
    } else {
        page->value_pte[index].pfn = ADDR_TO_PFN(paddr);
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
        page->value_pde[index].cache_disabled = disable_cache;
    }
}

void kernel_indirect_map_array(ulong vaddr, ulong paddr, size_t size, int disable_cache, int override)
{
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    ulong pstart = (paddr / PAGE_SIZE) * PAGE_SIZE;
    
    ulong page_count = size / PAGE_SIZE;
    if (size % PAGE_SIZE) {
        page_count++;
    }
    
    int i;
    ulong vcur = vstart;
    ulong pcur = pstart;
    for (i = 0; i < page_count; i++) {
        kernel_indirect_map(vcur, pcur, disable_cache, override);
        
        vcur += PAGE_SIZE;
        pcur += PAGE_SIZE;
    }
}

void kernel_direct_map(ulong addr, int disable_cache)
{
    kernel_indirect_map(addr, addr, disable_cache, 0);
}

void kernel_direct_map_array(ulong addr, size_t size, int disable_cache)
{
    ulong start = addr / PAGE_SIZE * PAGE_SIZE;
    ulong page_count = size / PAGE_SIZE;
    if (size % PAGE_SIZE) {
        page_count++;
    }
    
    ulong cur = start;
    int i;
    for (i = 0; i < page_count; i++) {
        kernel_direct_map(cur, disable_cache);
        cur += PAGE_SIZE;
    }
}
