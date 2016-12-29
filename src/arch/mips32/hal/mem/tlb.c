#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
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

void map_tlb_entry(int index, u32 vaddr, u32 pfn0, u32 pfn1, int write)
{
    struct tlb_entry tlb;
    
    tlb.hi.value = 0;
    tlb.hi.vpn2 = vaddr >> (PAGE_BITS + 1);
    
    tlb.pm.value = 0;
    tlb.pm.mask = 0;
    
    tlb.lo0.valid = 0;
        tlb.lo0.pfn = pfn0;
    tlb.lo0.valid = 1;
    tlb.lo0.dirty = write;
    tlb.lo0.coherent = 0x3;
    
    tlb.lo1.valid = 0;
    tlb.lo1.pfn = pfn1;
    tlb.lo1.valid = 1;
    tlb.lo1.dirty = write;
    tlb.lo1.coherent = 0x3;
    
    write_tlb_entry(index, tlb.hi.value, tlb.pm.value, tlb.lo0.value, tlb.lo1.value);
}

int tlb_probe(u32 addr)
{
    int index = -1;
    struct tlb_entry_hi hi;
    
    hi.value = 0;
    hi.vpn2 = addr >> (PAGE_BITS + 1);
    
    __asm__ __volatile__ (
        "mtc0   %1, $10;"   // hi
        "ehb;"
        "tlbp;"
        "mfc0   %0, $0;"    // index
        "nop"
        : "=r" (index)
        : "r" (hi.value)
    );
    
    return index;
}

void tlb_refill_kernel(u32 addr)
{
    struct tlb_entry tlb;
    
    tlb.hi.value = 0;
    tlb.hi.vpn2 = addr >> (PAGE_BITS + 1);
    
    tlb.pm.value = 0;
    tlb.pm.mask = 0xfff;
    
    tlb.lo0.valid = 0;
    tlb.lo0.pfn = ADDR_TO_PFN(addr) & ~0x1;
    tlb.lo0.valid = 1;
    tlb.lo0.dirty = 1;
    tlb.lo0.coherent = 0x3;
    
    tlb.lo1.valid = 0;
    tlb.lo1.pfn = ADDR_TO_PFN(addr) | 0x1;
    tlb.lo1.valid = 1;
    tlb.lo1.dirty = 1;
    tlb.lo1.coherent = 0x3;
    
    write_tlb_entry(-1, tlb.hi.value, tlb.pm.value, tlb.lo0.value, tlb.lo1.value);
}

void tlb_refill_user(u32 addr)
{
    struct page_frame *page = NULL;
    u32 page_addr = 0;
    struct tlb_entry tlb;
    
    // First map page dir
    int dir_index = tlb_probe(page_addr);
    assert(dir_index < 0);
    map_tlb_entry(-1, page_addr, ADDR_TO_PFN(page_addr) & ~0x1, ADDR_TO_PFN(page_addr) | 0x1, 0);
    dir_index = tlb_probe(page_addr);
    assert(dir_index >= 0);
    
    // Access the page dir
    u32 table_pfn = page->value_pde[GET_PDE_INDEX(addr)].pfn;
    page_addr = PFN_TO_ADDR(table_pfn);
    page = (struct page_frame *)page_addr;
    
    // Next map page table
    int table_index = tlb_probe(page_addr);
    assert(table_index < 0);
    map_tlb_entry(dir_index, page_addr, ADDR_TO_PFN(page_addr) & ~0x1, ADDR_TO_PFN(page_addr) | 0x1, 0);
    table_index = tlb_probe(page_addr);
    assert(table_index == dir_index);
    
    // Access the page table
    u32 page_pfn = page->value_pte[GET_PTE_INDEX(addr)].pfn;
    page_addr = PFN_TO_ADDR(page_pfn);
    
    // Finally map the actual page
    map_tlb_entry(dir_index, addr, ADDR_TO_PFN(page_addr) & ~0x1, ADDR_TO_PFN(page_addr) | 0x1, 0);
    assert(tlb_probe(page_addr) == dir_index);
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
