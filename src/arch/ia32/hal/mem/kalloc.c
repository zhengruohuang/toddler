#include "common/include/data.h"
#include "common/include/memory.h"
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
    
    //kprintf("PAlloc: %p, count: %d\n", result, count);
    
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
    mempool_limit += size;
    return result;
}

void init_kalloc()
{
    kprintf("Initializing HAL memory allocator ... ");
    
    mempool_limit = get_bootparam()->hal_vaddr_end;
    kprintf("mempool start addr: %p\n", mempool_limit);
}
