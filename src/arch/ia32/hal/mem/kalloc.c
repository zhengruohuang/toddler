#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"


static ulong mempool_limit = 0;

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
