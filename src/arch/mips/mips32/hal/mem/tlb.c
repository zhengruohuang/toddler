#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/mem.h"


static int tlb_entry_count = 0;
static int reserved_tlb_entry_count = 0;

dec_per_cpu(struct page_frame *, cur_page_dir);


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
        :
        : "r" (entry_index), "r" (hi), "r" (pm), "r" (lo0), "r" (lo1)
        : "memory"
    );
}

void map_tlb_entry_user(int index, u32 asid, u32 vaddr, u32 pfn0, int write0, u32 pfn1, int write1)
{
    struct tlb_entry tlb;
    struct tlb_entry_hi hi;
    
    __asm__ __volatile__ (
        "mfc0   %0, $10;"   // hi
        : "=r" (hi.value)
        :
    );
    
    tlb.hi.asid = asid;
    tlb.hi.vpn2 = vaddr >> (PAGE_BITS + 1);
    
    tlb.pm.value = 0;
    tlb.pm.mask = 0;
    
    tlb.lo0.value = 0;
    tlb.lo0.pfn = pfn0;
    tlb.lo0.valid = 1;
    tlb.lo0.dirty = write0;
    tlb.lo0.coherent = 0x6;
    
    tlb.lo1.value = 0;
    tlb.lo1.pfn = pfn1;
    tlb.lo1.valid = 1;
    tlb.lo1.dirty = write1;
    tlb.lo1.coherent = 0x6;
    
    write_tlb_entry(index, tlb.hi.value, tlb.pm.value, tlb.lo0.value, tlb.lo1.value);
    
    // Restore current ASID
    __asm__ __volatile__ (
        "mtc0   %0, $10;"   // hi
        :
        : "r" (hi.value)
    );
}

static void map_tlb_entry_kernel(u32 addr)
{
    // Use 256MB mask
    u32 mask = 0xffff;
    u32 physical_pfn = ADDR_TO_PFN(addr) & ~mask;
    
    struct tlb_entry tlb;
    
    tlb.hi.value = 0;
    tlb.hi.vpn2 = addr >> (PAGE_BITS + 1);
    
    tlb.pm.value = 0;
    tlb.pm.mask = mask;
    
    tlb.lo0.value = 0;
    tlb.lo0.pfn = physical_pfn;
    tlb.lo0.valid = 1;
    tlb.lo0.dirty = 1;
    tlb.lo0.coherent = 0x6;
    
    tlb.lo1.value = 0;
    tlb.lo1.pfn = physical_pfn | (mask + 0x1);
    tlb.lo1.valid = 1;
    tlb.lo1.dirty = 1;
    tlb.lo1.coherent = 0x6;
    
    write_tlb_entry(-1, tlb.hi.value, tlb.pm.value, tlb.lo0.value, tlb.lo1.value);
}

static int tlb_probe_kernel(u32 addr)
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
        : "=r" (index)
        : "r" (hi.value)
    );
    
    return index;
}

static u32 read_asid()
{
    struct tlb_entry_hi hi;
    
    __asm__ __volatile__ (
        "mfc0   %0, $10;"   // hi
        : "=r" (hi.value)
        :
    );
    
    return hi.asid;
}

static void set_asid(u32 asid)
{
    struct tlb_entry_hi hi;
    
    __asm__ __volatile__ (
        "mfc0   %0, $10;"   // hi
        : "=r" (hi.value)
        :
    );
    
    hi.asid = asid;
    
    __asm__ __volatile__ (
        "mtc0   %0, $10;"   // hi
        :
        : "r" (hi.value)
    );
}

int tlb_refill_kernel(u32 addr)
{
    map_tlb_entry_kernel(addr);
    return 0;
}

// Note that we are running this function in kernel mode, ASID is already set to be 0
int tlb_refill_user(u32 addr)
{
//     kprintf("User TLB refill @ %x ... ", addr);
    
    u32 user_asid = read_asid();
    set_asid(0);
    
    struct page_frame **cur_page_dir = get_per_cpu(struct page_frame *, cur_page_dir);
    struct page_frame *page = *cur_page_dir;
    u32 page_addr = (u32)page;
    struct tlb_entry tlb;
    
    // First map page dir
    int dir_index = tlb_probe_kernel(page_addr);
    if (dir_index < 0) {
        map_tlb_entry_kernel(page_addr);
    }
    dir_index = tlb_probe_kernel(page_addr);
    assert(dir_index >= 0);
    
    // Access the page dir
    u32 table_pfn = page->value_pde[GET_PDE_INDEX(addr)].pfn;
    page_addr = PFN_TO_ADDR(table_pfn);
    page = (struct page_frame *)page_addr;
    
    // Next map page table
    int table_index = tlb_probe_kernel(page_addr);
    if (table_index < 0) {
        map_tlb_entry_kernel(page_addr);
    }
    dir_index = tlb_probe_kernel(page_addr);
    assert(dir_index >= 0);
    
    // Access the page table
    u32 pte_index0 = GET_PTE_INDEX(addr) & ~0x1;
    u32 page_pfn0 = page->value_pte[pte_index0].pfn;
    int write0 = page->value_pte[pte_index0].write_allow;
    
    u32 pte_index1 = pte_index0 | 0x1;
    u32 page_pfn1 = page->value_pte[pte_index1].pfn;
    int write1 = page->value_pte[pte_index1].write_allow;
    
    // Finally map the actual page
    map_tlb_entry_user(dir_index, user_asid, addr, page_pfn0, write0, page_pfn1, write1);
    
    // Restore ASID
    set_asid(user_asid);
    
//     kprintf("done!\n");
    
    return 0;
}

static no_opt void invalidate_tlb(ulong asid, ulong vaddr)
{
//     kprintf("TLB shootdown @ %x, ASID: %x ...", vaddr, asid);
    
    int index = -1;
    struct tlb_entry_hi hi;
    ulong hi_old = 0;
    
    // Read the old value of HI
    __asm__ __volatile__ (
        "mfc0   %0, $10;"   // hi
        : "=r" (hi.value)
        :
    );
    hi_old = hi.value;
    
    // Set ASID and Vaddr
    hi.asid = asid;
    hi.vpn2 = vaddr >> (PAGE_BITS + 1);
    
    // Write HI and do a TLB probe
    __asm__ __volatile__ (
        "mtc0   %1, $10;"   // hi
        "ehb;"
        "tlbp;"
        "mfc0   %0, $0;"    // index
        : "=r" (index)
        : "r" (hi.value)
    );
    
//     kprintf(" index @ %x ...", index);
    
    // If there's no match then there's no need to do an invalidation
    if (index >= 0) {
        // Write to TLB to do an invalidation
        __asm__ __volatile__ (
            "mtc0   $0, $10;"       // hi
            "mtc0   $0, $5;"        // pm
            "mtc0   $0, $2;"        // lo0
            "mtc0   $0, $3;"        // lo1
            "mtc0   %0, $0;"        // index
            "ehb;"                  // clear hazard barrier
            "tlbwi;"                // write indexed entry
            :
            : "r" (index)
            : "memory"
        );
    }
    
    // Restore old HI
    __asm__ __volatile__ (
        "mtc0   %0, $10;"   // hi
        :
        : "r" (hi_old)
    );
    
//     kprintf(" done");
}

void invalidate_tlb_array(ulong asid, ulong vaddr, size_t size)
{
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    
    ulong page_count = size / PAGE_SIZE;
    if (size % PAGE_SIZE) {
        page_count++;
    }
    
    ulong i;
    ulong vcur = vstart;
    for (i = 0; i < page_count; i++) {
        invalidate_tlb(asid, vcur);
        vcur += PAGE_SIZE;
    }
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
