#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"


static ulong alloc_init_kernel_pte()
{
    ulong result = PFN_TO_ADDR((ulong)get_bootparam()->free_pfn_start);
    get_bootparam()->free_pfn_start++;
    return result;
}

void kernel_indirect_map(ulong vaddr, ulong paddr, int disable_cache)
{
    // PDE
    struct page_frame *page = (struct page_frame *)KERNEL_PDE_PADDR;
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        page->value_pde[index].pfn = alloc_init_kernel_pte();
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (page->value_u32[index]) {
        assert(
            page->value_pte[index].pfn == ADDR_TO_PFN(paddr) &&
            page->value_pde[index].present &&
            page->value_pde[index].rw &&
            page->value_pde[index].cache_disabled == disable_cache
        );
    } else {
        page->value_pte[index].pfn = ADDR_TO_PFN(paddr);
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
        page->value_pde[index].cache_disabled = disable_cache;
    }
}

void kernel_indirect_map_array(ulong vaddr, ulong paddr, size_t size, int disable_cache)
{
}

void kernel_direct_map(ulong addr, int disable_cache)
{
    kernel_indirect_map(addr, addr, disable_cache);
}

void kernel_direct_map_array(ulong addr, size_t size, int disable_cache)
{
    ulong start = addr;
    ulong end = addr + size;
    
    if (start % sizeof(ulong)) {
        start /= sizeof(ulong);
        start *= sizeof(ulong);
    }
    
    if (end % sizeof(ulong)) {
        end /= sizeof(ulong);
        end++;
        end *= sizeof(ulong);
    }
    
    ulong cur;
    for (cur = start; cur < end; cur += sizeof(ulong)) {
        kernel_direct_map(cur, disable_cache);
    }
}
