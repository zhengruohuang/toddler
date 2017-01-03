#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"


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
    
//     page->value_pde[0].pfn = KERNEL_PTE_LO4_PFN;
//     page->value_pde[0].present = 1;
//     page->value_pde[0].rw = 1;
//     page->value_pde[0].user = 0;
//     page->value_pde[0].cache_disabled = 0;
}


/*
 * Get physical address of a user virtual address
 */
ulong get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    // PDE
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        return 0;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        return 0;
    }
    
    // Paddr
    ulong paddr = PFN_TO_ADDR(page->value_pte[index].pfn);
    paddr += vaddr % PAGE_SIZE;
    
    return paddr;
}


/*
 * User memory mapping
 */
static int user_indirect_map(
    ulong page_dir_pfn, ulong vaddr, ulong paddr,
    int exec, int write, int cacheable, int override)
{
    // PDE
    volatile struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        ulong alloc_pfn = 0;    // FIXME: kernel->palloc(1);;
        if (!alloc_pfn) {
            return 0;
        }
        
        // FIXME: the page should've been zeroed by kernel
        memzero((void *)PFN_TO_ADDR(alloc_pfn), PAGE_SIZE);

        page->value_pde[index].pfn = alloc_pfn;
        page->value_pde[index].present = 1;
    }
    
    if (!page->value_pde[index].present) {
        return 0;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
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
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    ulong pstart = (paddr / PAGE_SIZE) * PAGE_SIZE;
    
    ulong page_count = length / PAGE_SIZE;
    if (length % PAGE_SIZE) {
        page_count++;
    }
    
    ulong i;
    ulong vcur = vstart;
    ulong pcur = pstart;
    for (i = 0; i < page_count; i++) {
        int succeed = user_indirect_map(page_dir_pfn, vcur, pcur, exec, write, cacheable, overwrite);
        
        if (!succeed) {
            return 0;
        }
        
        vcur += PAGE_SIZE;
        pcur += PAGE_SIZE;
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
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    assert(page->value_u32[index]);
    
    // Unmap
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
        // FIXME: assert(kernel->pfree(ADDR_TO_PFN((ulong)page)));
    }
    
    return 1;
}

int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length)
{
    //kprintf("To unmap, pfn: %u, vaddr: %u, paddr: %u, size: %u\n", page_dir_pfn, vaddr, paddr, length);
    
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    ulong pstart = (paddr / PAGE_SIZE) * PAGE_SIZE;
    
    ulong page_count = length / PAGE_SIZE;
    if (length % PAGE_SIZE) {
        page_count++;
    }
    
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
