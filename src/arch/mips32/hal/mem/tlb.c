#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"


static int tlb_entry_count = 0;
static int reserved_tlb_entry_count = 0;


int reserve_tlb_entry()
{
    if (reserved_tlb_entry_count >= tlb_entry_count) {
        return -1;
    }
    
    // Set wired limit
    __asm__ __volatile__ (
        "mtc0   %0, $6;"        // wired
        :
        : "r" (reserved_tlb_entry_count)
    );
    
    return reserved_tlb_entry_count++;
}

void write_tlb_entry(int index, u32 hi, u32 pm, u32 lo0, u32 lo1)
{
    u32 entry_index = index;
    
    // Find a random entry
    if (index < 0) {
        __asm__ __volatile__ (
            "mfc0   %0, $1, 0;" // Read random register - CP0 reg 1, sel 0
            : "=r" (entry_index)
            :
        );
    }
    
    __asm__ __volatile__ (
        "mtc0   %1, $10;"       // hi
        "mtc0   %2, $5;"        // pm
        "mtc0   %3, $2;"        // lo0
        "mtc0   %4, $3;"        // lo1
        "mtc0   %0, $0;"        // index
        "ehb;"                  // clear hazard barrier
        "tlbwi;"                // write indexed entry
        "nop;"                  // have a rest
        :
        : "r" (entry_index), "r" (hi), "r" (pm), "r" (lo0), "r" (lo1)
        : "memory"
    );
}

void init_tlb()
{
    int i;
    
    // Get TLB entry count
    __asm__ __volatile__ (
        "mfc0   %0, $16, 1;"    // Read config reg - CP0 reg 16 sel 1
        : "=r" (tlb_entry_count)
        :
    );
    
    tlb_entry_count >>= 25;
    tlb_entry_count &= 0x3F;
    tlb_entry_count += 1;
    
    // Set wired limit to zero
    __asm__ __volatile__ (
        "mtc0   $0, $6;"        // wired - 0
        :
        :
    );
    
    // Zero all TLB entries
    for (i = 0; i < tlb_entry_count; i++) {
        write_tlb_entry(i, 0, 0, 0, 0);
    }
    
    kprintf("TLB initialized, entry count: %d\n", tlb_entry_count);
}
