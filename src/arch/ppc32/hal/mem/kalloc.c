#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"


static ulong mempool_limit = 0;


/*
 * Allocate a page, returns PFN
 */
ulong palloc(int count)
{
    assert(count > 0);
    
    ulong result = (ulong)get_bootparam()->free_pfn_start;
    get_bootparam()->free_pfn_start += count;
    
    // Map the page
    fill_kernel_pht(PFN_TO_ADDR(result), count << PAGE_BITS, 0, 0);
    
    // Zero the page
    memzero((void *)PFN_TO_ADDR(result), count << PAGE_BITS);
    
    //kprintf("PAlloc: %p, count: %d\n", result, count);
    
    return result;
}


void kfree(void *ptr)
{
    warn("HAL memory allocator does not support free @ %p, request ignored\n", ptr);
}

void *kalloc(size_t size)
{
    assert(mempool_limit > 0);
    
    void *result = (void *)mempool_limit;
    mempool_limit += size;
    
    kprintf("\tMempool alloced @ %p, avail range @ %p to %p, allocatable size @ %d KB\n",
            result, (void *)mempool_limit, (void *)get_bootparam()->hal_vspace_end,
            (int)(get_bootparam()->hal_vspace_end - mempool_limit) / 1024
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
            (int)(get_bootparam()->hal_vspace_end - mempool_limit) / 1024
    );
}
