#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/reg.h"
#include "common/include/atomic.h"
#include "hal/include/print.h"
#include "hal/include/bit.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/mem.h"


static void invalidate_tlb(ulong asid, ulong vaddr)
{
//     kprintf("TLB shootdown @ %lx, ASID: %lx ...", vaddr, asid);
    inv_tlb_all();
    atomic_membar();
}

void invalidate_tlb_array(ulong asid, ulong vaddr, size_t size)
{
    ulong vstart = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong vend = ALIGN_UP(vaddr + size, PAGE_SIZE);
    ulong page_count = (vend - vstart) >> PAGE_BITS;
    
    ulong i;
    ulong vcur = vstart;
    for (i = 0; i < page_count; i++) {
        invalidate_tlb(asid, vcur);
        vcur += PAGE_SIZE;
    }
}

void init_tlb()
{

}
