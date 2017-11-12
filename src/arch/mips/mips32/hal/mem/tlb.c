#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/reg.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/mem.h"


static int has_vtlb = 0;
static u32 vtlb_size_mask = 0;
static int vtlb_entry_count;

static int has_ftlb = 0;
static u32 ftlb_ways = 0;
static u32 ftlb_sets = 0;
static int ftlb_entry_count = 0;

static int tlb_entry_count = 0;

dec_per_cpu(struct page_frame *, cur_page_dir);


#define write_tlb_indexed()                     \
    __asm__ __volatile__ (                      \
        "ehb;"      /* clear hazard barrier */  \
        "tlbwi;"    /* write indexed entry */   \
        : :                                     \
    )

#define tlb_probe()         \
    __asm__ __volatile__ (  \
        "ehb;"              \
        "tlbp;"             \
        : :                 \
    )


void write_tlb_entry(int index, int nonstd, ulong hi, ulong pm, ulong lo0, ulong lo1)
{
    u32 entry_index = index;
    
    // Find a random entry
    if (index < 0) {
        read_cp0_random(entry_index);
        
        // Limit the index to VTLB if needed
        if (nonstd) {
            assert(has_vtlb);
            entry_index &= vtlb_size_mask;
        }
    }
    
    // Write to TLB
    write_cp0_entry_hi(hi);
    write_cp0_page_mask(pm);
    write_cp0_entry_lo0(lo0);
    write_cp0_entry_lo1(lo1);
    write_cp0_index(entry_index);
    
    // Apply the changes
    write_tlb_indexed();
}

void map_tlb_entry_user(int index, ulong asid, ulong vaddr,
                        int valid0, ulong pfn0, int write0,
                        int valid1, ulong pfn1, int write1)
{
    struct cp0_entry_hi hi;
    struct cp0_page_mask pm;
    
    struct cp0_entry_lo lo0;
    struct cp0_entry_lo lo1;
    
    hi.value = 0;
    hi.asid = asid;
    hi.vpn2 = vaddr >> TLB_VPN2_SHIFT_BITS;
    
    pm.value = 0;
    pm.mask_ex = USER_PAGE_MASK_EX;
    pm.mask = USER_PAGE_MASK;
    
    lo0.value = 0;
    if (valid0) {
        assert(pfn0);
        lo0.valid = valid0;
        lo0.pfn = pfn0;
        lo0.dirty = write0;
        lo0.coherent = 0x6;
    }
    
    lo1.value = 0;
    if (valid1) {
        assert(pfn1);
        lo1.valid = valid1;
        lo1.pfn = pfn1;
        lo1.dirty = write1;
        lo1.coherent = 0x6;
    }
    
    write_tlb_entry(index, 0, hi.value, pm.value, lo0.value, lo1.value);
}

static void map_tlb_entry_kernel(u32 addr)
{
    // Use 256MB mask
    //u32 mask = 0xffff;
    ulong mask = ((KERNEL_PAGE_MASK) << 12) | (KERNEL_PAGE_MASK_EX << 10) | 0x3ff;
    ulong physical_pfn = ADDR_TO_PFN(addr) & ~mask;
    physical_pfn >>= 12;
    
    struct cp0_entry_hi hi;
    struct cp0_page_mask pm;
    
    struct cp0_entry_lo lo0;
    struct cp0_entry_lo lo1;
    
    hi.value = 0;
    hi.vpn2 = addr >> TLB_VPN2_SHIFT_BITS;
    
    pm.value = 0;
    pm.mask_ex = KERNEL_PAGE_MASK_EX;
    pm.mask = KERNEL_PAGE_MASK;
    
    lo0.value = 0;
    lo0.pfn = physical_pfn & ~0x1ul;
    lo0.valid = 1;
    lo0.dirty = 1;
    lo0.coherent = 0x6;
    
    lo1.value = 0;
    lo1.pfn = physical_pfn | 0x1ul;
    lo1.valid = 1;
    lo1.dirty = 1;
    lo1.coherent = 0x6;
    
    write_tlb_entry(-1, 1, hi.value, pm.value, lo0.value, lo1.value);
}

static no_opt int probe_tlb_index(ulong asid, ulong vaddr)
{
    int index = -1;
    struct cp0_entry_hi hi, hi_old;
    
    // Read the old value of HI
    read_cp0_entry_hi(hi.value);
    hi_old.value = hi.value;
    
    // Set ASID and Vaddr
    hi.asid = asid;
    hi.vpn2 = vaddr >> TLB_VPN2_SHIFT_BITS;
    
    // Write HI and do a TLB probe
    write_cp0_entry_hi(hi.value);
    tlb_probe();
    read_cp0_index(index);
    
    // Restore old HI
    write_cp0_entry_hi(hi_old.value);
    
    return index;
}

static int tlb_probe_kernel(ulong addr)
{
    return probe_tlb_index(0, addr);
}

static ulong read_asid()
{
    struct cp0_entry_hi hi;
    read_cp0_entry_hi(hi.value);
    return hi.asid;
}

static void set_asid(ulong asid)
{
    struct cp0_entry_hi hi;
    
    read_cp0_entry_hi(hi.value);
    hi.asid = asid;
    write_cp0_entry_hi(hi.value);
}

int tlb_refill_kernel(ulong addr)
{
    map_tlb_entry_kernel(addr);
    return 0;
}

// Note that we are running this function in kernel mode, ASID is already set to be 0
int tlb_refill_user(ulong addr)
{
//     kprintf("User TLB refill @ %x ... ", addr);
    
    // Align the addr to double-page boundary
    addr &= ~((PAGE_SIZE << 1) - 1);
    
    // User's ASID
    ulong user_asid = read_asid();
    set_asid(0);
    
    struct page_frame **cur_page_dir = get_per_cpu(struct page_frame *, cur_page_dir);
    struct page_frame *page = *cur_page_dir;
    ulong page_addr = (ulong)page;
    
    // First map page dir
    int dir_index = tlb_probe_kernel(page_addr);
    if (dir_index < 0) {
        map_tlb_entry_kernel(page_addr);
    }
    dir_index = tlb_probe_kernel(page_addr);
    assert(dir_index >= 0);
    
    // Access the page dir
    ulong table_pfn = page->value_pde[GET_PDE_INDEX(addr)].pfn;
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
    int pte_index0 = GET_PTE_INDEX(addr) & ~0x1;
    int valid0 = page->value_pte[pte_index0].present;
    ulong pfn0 = page->value_pte[pte_index0].pfn;
    int write0 = page->value_pte[pte_index0].write_allow;
    
    int pte_index1 = pte_index0 | 0x1;
    int valid1 = page->value_pte[pte_index1].present;
    ulong pfn1 = page->value_pte[pte_index1].pfn;
    int write1 = page->value_pte[pte_index1].write_allow;

__do_write:
    // Fix PFN bits
    pfn0 = (pfn0 << PAGE_BITS) >> TLB_PFN_SHIFT_BITS;
    pfn1 = (pfn1 << PAGE_BITS) >> TLB_PFN_SHIFT_BITS;
    
//     kprintf("User PFN @ %lx, %lx, valid: %d, %d\n", pfn0, pfn1, valid0, valid1);
    
    // Finally map the actual page
    int tlb_index = probe_tlb_index(user_asid, addr);
    map_tlb_entry_user(tlb_index, user_asid, addr,
                       valid0, pfn0, write0, valid1, pfn1, write1);

    // Restore ASID
    set_asid(user_asid);
    
//     kprintf("done!\n");
    
    return 0;
}


/*
 * TLB Invalidation
 */
static void write_invalid_vtlb_entry(int index)
{
    write_tlb_entry(index, 1, 0, 0, 0, 0);
}

static void write_invalid_ftlb_entry(int index)
{
    write_tlb_entry(index, 0, 0, 0, 0, 0);
}

static void invalidate_tlb(ulong asid, ulong vaddr)
{
    //kprintf("TLB shootdown @ %lx, ASID: %lx ...", vaddr, asid);
    
    int index = probe_tlb_index(asid, vaddr);
    
    // If there's no match then there's no need to do an invalidation
    if (index >= 0) {
        if (has_vtlb && (index < vtlb_entry_count)) {
            write_invalid_vtlb_entry(index);
        } else if (has_ftlb && (index >= vtlb_entry_count && index < tlb_entry_count)) {
            write_invalid_ftlb_entry(index);
        } else {
            panic("Unable to locate TLB index @ %d\n", index);
        }
    }
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


/*
 * Init
 */
static void flush_tlb()
{
    int i;
    
    if (has_vtlb) {
        for (i = 0; i < vtlb_entry_count; i++) {
            write_invalid_vtlb_entry(i);
        }
    }
    
    if (has_ftlb) {
        for (i = vtlb_entry_count; i < tlb_entry_count; i++) {
            write_invalid_ftlb_entry(i);
        }
    }
}

static void init_vtlb()
{
    u32 val = 0;
    struct cp0_config0 c0;
    struct cp0_config1 c1;
    struct cp0_config4 c4;
    
    // See if the processor has VTLB
    read_cpu_config(0, &val);
    c0.value = val;
    
    if (c0.mmu_type != 1 && c0.mmu_type != 4) {
        has_vtlb = 0;
        vtlb_size_mask = 0;
        kprintf("Processor does not have VTLB!\n");
        return;
    }
    has_vtlb = 1;
    
    vtlb_size_mask = 0x3f;
    
    //Find out how many VTLB entries are present
    read_cpu_config(1, &val);
    c1.value = val;
    vtlb_size_mask = c1.vtlb_size;
    
    // Size extension
    int success = read_cpu_config(4, &val);
    if (success) {
        c4.value = val;
        
        if (c0.arch_rev <= 1 && c4.mmu_ext_type != 1) {
            vtlb_size_mask |= c4.vtlb_size_ex << 6;
        } else {
            vtlb_size_mask |= c4.mmu_size_ext << 6;
        }
    }
    
    // Entry count
    vtlb_entry_count = vtlb_size_mask + 1;
    
    // Set wired limit to zero
    write_cp0_wired(0);
    
    kprintf("VTLB initialized, count: %d, size mask: %x\n",
            vtlb_entry_count, vtlb_size_mask);
}

static void init_ftlb()
{
    u32 val = 0;
    struct cp0_config0 c0;
    struct cp0_config4 c4;
    
    // See if the processor has FTLB
    assert(read_cpu_config(0, &val));
    c0.value = val;
    
    if (c0.mmu_type != 4) {
        has_ftlb = 0;
        ftlb_ways = ftlb_sets = 0;
        kprintf("Processor does not have FTLB!\n");
        return;
    }
    has_ftlb = 1;
    
    // Find out how many FTLB entries are present
    assert(read_cpu_config(4, &val));
    c4.value = val;
    
    ftlb_ways = c4.ftlb_ways;
    ftlb_sets = c4.ftlb_sets;
    
    // Set page size to 4KB
    c4.ftlb_page = 1;
    write_cp0_config(4, c4.value);
    
    // Verify that the processor supports our page size
    // and do not use FTLB in case the page size is not supported
    assert(read_cpu_config(4, &val));
    if (c4.value != val) {
        has_ftlb = 0;
        ftlb_ways = ftlb_sets = 0;
        kprintf("FTLB does not support 4KB page size!\n");
        return;
    }
    
    // Entry count
    ftlb_entry_count = ftlb_sets * ftlb_ways;
    
    kprintf("FTLB initialized, ways: %u, sets: %u, entry count: %x\n",
            ftlb_ways, ftlb_sets, ftlb_entry_count);
}

void init_tlb()
{
    // Init all
    init_vtlb();
    init_ftlb();
    
    // Total TLB entry count
    tlb_entry_count = vtlb_entry_count + ftlb_entry_count;
    
    // Flush TLB
    flush_tlb();
    
    // Set up page grain config
    struct cp0_page_grain grain;
    read_cp0_page_grain(grain.value);
    grain.value = 0;
    grain.iec = 1;
    grain.xie = 1;
    grain.rie = 1;
    write_cp0_page_grain(grain.value);
    
    kprintf("TLB initialized, entry count: %d\n", tlb_entry_count);
}
