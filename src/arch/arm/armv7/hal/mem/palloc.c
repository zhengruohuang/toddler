#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "hal/include/print.h"
#include "hal/include/bootparam.h"
#include "hal/include/debug.h"
#include "hal/include/string.h"


/*
 * Allocate a page, returns PFN
 */
ulong palloc(int count)
{
    assert(count > 0);
    
    struct boot_parameters *bp = get_bootparam();
    
    ulong result = bp->free_pfn_start;
    bp->free_pfn_start += count;
    
    // Zero the page
    memzero((void *)PFN_TO_ADDR(result), PAGE_SIZE * count);
    
//     kprintf("PAlloc: %lx, count: %d\n", result, count);
    
    return result;
}
