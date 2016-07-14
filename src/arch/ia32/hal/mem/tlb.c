#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/mem.h"


static no_opt void invalidate_tlb(ulong vaddr)
{
    __asm__ __volatile__
    (
        "invlpg (%%eax);"
        :
        : "a" (vaddr)
        : "memory"
    );
}

void invalidate_tlb_array(ulong vaddr, size_t size)
{
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    
    ulong page_count = size / PAGE_SIZE;
    if (size % PAGE_SIZE) {
        page_count++;
    }
    
    ulong i;
    ulong vcur = vstart;
    for (i = 0; i < page_count; i++) {
        invalidate_tlb(vcur);
        vcur += PAGE_SIZE;
    }
}
