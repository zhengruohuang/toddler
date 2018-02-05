#include "common/include/data.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"


static no_opt void invalidate_tlb(ulong asid, ulong vaddr)
{
//     kprintf("TLB shootdown @ %x, ASID: %x ...", vaddr, asid);
    
    evict_pht(asid, vaddr);
    
    __asm__ __volatile__
    (
        "tlbie %[addr];"
        "eieio;"
        "tlbsync;"
        "sync;"
        :
        : [addr]"r"(vaddr)
    );
}

void invalidate_tlb_array(ulong asid, ulong vaddr, size_t size)
{
    ulong vstart = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong vend = ALIGN_UP(vaddr + size, PAGE_SIZE);
    ulong page_count = (vend - vstart) >> PAGE_BITS;
    
//     kprintf("Invalidate TLB array, vaddr @ %lx, size: %lx, vstart @ %lx, vend @ %lx, pages: %lx\n",
//         vaddr, size, vstart, vend, page_count
//     );
    
    ulong i;
    ulong vcur = vstart;
    for (i = 0; i < page_count; i++) {
        invalidate_tlb(asid, vcur);
        vcur += PAGE_SIZE;
    }
}
