#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"


static ulong mempool_limit = 0;


/*
 * Allocate a page, returns PFN
 */
ulong palloc(int count)
{
    assert(count > 0);
    
    ulong result = get_bootparam()->free_pfn_start;
    get_bootparam()->free_pfn_start += count;
//     assert(result >= KERNEL_INIT_PTE_START_PFN);
    
    // Zero the page
    memzero((void *)PHYS_TO_KCODE(PFN_TO_ADDR(result)), PAGE_SIZE * count);
    
//     kprintf("PAlloc: %lx, count: %d\n", result, count);
    
    return result;
}


void kfree(void *ptr)
{
    warn("HAL memory allocator does not support free, request ignored\n");
}

void *kalloc(size_t size)
{
    assert(mempool_limit > 0);
    
    void *result = (void *)mempool_limit;
    mempool_limit += ALIGN_UP(size, sizeof(ulong));
    
    kprintf("\tMempool alloced @ %p, avail range @ %p to %p, allocatable size: %d KB\n",
            result, (void *)mempool_limit, (void *)get_bootparam()->hal_vspace_end,
            (int)((get_bootparam()->hal_vspace_end - mempool_limit) >> 10)
    );
    
    assert(mempool_limit <= get_bootparam()->hal_vspace_end);
    
    return result;
}

void init_kalloc()
{
    kprintf("Initializing HAL memory allocator\n");
    
    mempool_limit = get_bootparam()->hal_vaddr_end;
    
    kprintf("\tMempool range @ %p to %p, allocatable size: %d KB\n",
            (void *)mempool_limit, (void *)get_bootparam()->hal_vspace_end,
            (int)((get_bootparam()->hal_vspace_end - mempool_limit) >> 10)
    );
}
