#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/print.h"
#include "hal/include/bootparam.h"
#include "hal/include/debug.h"
#include "hal/include/string.h"
#include "hal/include/bit.h"



static ulong mempool_limit = 0;


void kfree(void *ptr)
{
    warn("HAL memory allocator does not support free, request ignored\n");
}

void *kalloc(size_t size)
{
    assert(mempool_limit > 0);
    struct boot_parameters *bp = get_bootparam();
    
    void *result = (void *)mempool_limit;
    mempool_limit += ALIGN_UP(size, sizeof(ulong));
    
    kprintf("\tMempool alloced @ %p, avail range @ %p to %p, allocatable size: %d KB\n",
            result, (void *)mempool_limit, (void *)bp->hal_vspace_end,
            (int)((bp->hal_vspace_end - mempool_limit) >> 10)
    );
    
    assert(mempool_limit <= bp->hal_vspace_end);
    
    return result;
}

void init_kalloc()
{
    kprintf("Initializing HAL memory allocator\n");
    
    struct boot_parameters *bp = get_bootparam();
    mempool_limit = bp->hal_vaddr_end;
    
    kprintf("\tMempool range @ %p to %p, allocatable size: %d KB\n",
            (void *)mempool_limit, (void *)bp->hal_vspace_end,
            (int)((bp->hal_vspace_end - mempool_limit) >> 10)
    );
}
