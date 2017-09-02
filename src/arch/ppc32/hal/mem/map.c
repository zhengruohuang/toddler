#include "common/include/data.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/debug.h"
#include "hal/include/kernel.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"


static volatile struct page_frame *kernel_pde;
static volatile struct page_frame *kernel_pte;


/*
 * Initialize user page directory
 */
void init_user_page_dir(ulong page_dir_pfn)
{
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);

    int i;
    for (i = 0; i < 1024; i++) {
        page->value_u32[i] = 0;
    }
    
    
    page->value_pde[1023].pfn = ADDR_TO_PFN((ulong)kernel_pte);
    page->value_pde[1023].present = 1;
    page->value_pde[1023].supervisor = 1;
    page->value_pde[1023].next_level = 1;
}


/*
 * Get physical address of a user virtual address
 */
ulong get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    ulong paddr = -1;
    
    // PDE
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        //kprintf("Failed to get paddr PDE @ %p\n", (void *)vaddr);
        return 0;
    }
    
    // Has next level page table
    if (page->value_pde[index].next_level) {
        // Get PTE
        page = (struct page_frame *)(PFN_TO_ADDR((ulong)page->value_pde[index].pfn));
        index = GET_PTE_INDEX(vaddr);
        
        if (!page->value_u32[index]) {
            //kprintf("Failed to get paddr PTE @ %p\n", (void *)vaddr);
            return 0;
        }
        
        // Paddr
        paddr = PFN_TO_ADDR(page->value_pte[index].pfn);
        paddr += vaddr % PAGE_SIZE;
        
        return paddr;
    }
    
    // Does not have next level page table
    else {
        ulong ppfn = page->value_pde[index].pfn;
        ppfn += ADDR_TO_PFN(vaddr & 0x3FFFFF);
        
        paddr = PFN_TO_ADDR(ppfn);
    }
    
    // Done
    return paddr;
}


/*
 * User mapping
 */
static int user_indirect_map(
    ulong page_dir_pfn, ulong vaddr, ulong paddr,
    int exec, int write, int cacheable, int override)
{
    // PDE
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        ulong alloc_pfn = kernel->palloc(1);;
        if (!alloc_pfn) {
            return 0;
        }
        
        // FIXME: the page should've been zeroed by kernel
        memzero((void *)PFN_TO_ADDR(alloc_pfn), PAGE_SIZE);

        page->value_pde[index].pfn = alloc_pfn;
        page->value_pde[index].present = 1;
        page->value_pde[index].next_level = 1;
    }
    
    if (!page->value_pde[index].present) {
        return 0;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR((ulong)page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (page->value_u32[index]) {
        if (
            page->value_pte[index].pfn != ADDR_TO_PFN(paddr) ||
            !page->value_pde[index].present
        ) {
            //kprintf("Old PFN: %p, new PFN: %p, present: %d, user: %d\n", page->value_pte[index].pfn, ADDR_TO_PFN(paddr), page->value_pde[index].present, page->value_pde[index].user);
            
            return 0;
        }
        
        if (override) {
            page->value_pde[index].exec_allow = exec;
            page->value_pde[index].write_allow = write;
            page->value_pde[index].cache_allow = cacheable;
        } else  if (
            page->value_pde[index].exec_allow != exec ||
            page->value_pde[index].write_allow != write ||
            page->value_pde[index].cache_allow != cacheable
        ) {
            return 0;
        }
    }
    
    // This is our first time mapping the page
    else {
        page->value_pte[index].pfn = ADDR_TO_PFN(paddr);
        page->value_pde[index].present = 1;
        page->value_pde[index].exec_allow = exec;
        page->value_pde[index].write_allow = write;
        page->value_pde[index].cache_allow = cacheable;
    }
    
    return 1;
}

int user_indirect_map_array(
    ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length,
    int exec, int write, int cacheable, int overwrite)
{
    ulong vstart = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong pstart = ALIGN_DOWN(paddr, PAGE_SIZE);
    ulong vend = ALIGN_UP(vaddr + length, PAGE_SIZE);
    ulong page_count = (vend - vstart) >> PAGE_BITS;
    
//     kprintf("To map vaddr @ %p to %p, pfn @ %p\n",
//            (void *)vstart, (void *)vend, (void *)page_dir_pfn);
    
    ulong i;
    ulong vcur = vstart;
    ulong pcur = pstart;
    for (i = 0; i < page_count; i++) {
        int succeed = user_indirect_map(page_dir_pfn, vcur, pcur, exec, write, cacheable, overwrite);
        
        if (!succeed) {
            return 0;
        }
        
        pcur += PAGE_SIZE;
        vcur += PAGE_SIZE;
    }
    
    return 1;
}

static int user_indirect_unmap(ulong page_dir_pfn, ulong vaddr, ulong paddr)
{
    //kprintf("Doing unmap!\n");
    
    int need_free = 1;
    
    // PDE
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_PDE_INDEX(vaddr);
    assert(page->value_u32[index]);
    
    // PTE
    page = (struct page_frame *)((ulong)PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    assert(page->value_u32[index]);
    
    // Unmap
//     if (page->value_pte[index].pfn != ADDR_TO_PFN(paddr)) {
//         kprintf("Vaddr @ %p, Paddr @ %p\n", (void *)vaddr, (void *)paddr);
//         kprintf("page->value_pte[index].pfn: %p\n", (void *)(ulong)page->value_pte[index].pfn);
//         kprintf("ADDR_TO_PFN(paddr): %p\n", (void *)ADDR_TO_PFN(paddr));
//     }
    assert(page->value_pte[index].pfn == ADDR_TO_PFN(paddr));
    page->value_u32[index] = 0;
    
    // Free PTE page
    for (index = 0; index < PAGE_ENTRY_COUNT; index++) {
        if (page->value_u32[index]) {
            need_free = 0;
            break;
        }
    }
    
    if (need_free) {
        assert(kernel->pfree(ADDR_TO_PFN((ulong)page)));
    }
    
    return 1;
}

int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length)
{
//     kprintf("To unmap, pfn: %p, vaddr: %p, paddr: %p, size: %p\n",
//             (void *)page_dir_pfn, (void *)vaddr, (void *)paddr, (void *)length);
    
    ulong vstart = ALIGN_DOWN(vaddr, PAGE_SIZE);
    ulong pstart = ALIGN_DOWN(paddr, PAGE_SIZE);
    ulong vend = ALIGN_UP(vaddr + length, PAGE_SIZE);
    ulong page_count = (vend - vstart) >> PAGE_BITS;
    
    ulong i;
    ulong vcur = vstart;
    ulong pcur = pstart;
    for (i = 0; i < page_count; i++) {
        int succeed = user_indirect_unmap(page_dir_pfn, vcur, pcur);
        
        if (!succeed) {
            return 0;
        }
        
        vcur += PAGE_SIZE;
        pcur += PAGE_SIZE;
    }
    
    return 1;
}


/*
 * Kernel mapping
 */
void kernel_map_per_cpu_area(ulong vstart, ulong pstart, ulong size)
{
    ulong vend = ALIGN_UP(vstart + size, PAGE_SIZE);
    ulong vaddr = ALIGN_DOWN(vstart, PAGE_SIZE);
    ulong paddr = ALIGN_DOWN(pstart, PAGE_SIZE);
    
    for (; vaddr < vend; vaddr += PAGE_SIZE, paddr += PAGE_SIZE) {
        int pde_idx = GET_PDE_INDEX(vaddr);
        int pte_idx = GET_PTE_INDEX(vaddr);
        
        assert(kernel_pde->value_pde[pde_idx].next_level);
        
        kernel_pte->value_pte[pte_idx].present = 1;
        kernel_pte->value_pte[pte_idx].pfn = ADDR_TO_PFN(paddr);
    }
}

void init_map()
{
    struct boot_parameters *bp = get_bootparam();
    
    kernel_pde = (struct page_frame *)bp->pde_addr;
    kernel_pte = (struct page_frame *)bp->pte_addr;
}
