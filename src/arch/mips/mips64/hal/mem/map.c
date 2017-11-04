#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/kernel.h"
#include "hal/include/mem.h"


/*
 * Initialize user page directory
 */
void init_user_page_dir(ulong page_dir_pfn)
{
    volatile struct page_frame *page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR(page_dir_pfn));

    int i;
    for (i = 0; i < PAGE_ENTRY_COUNT; i++) {
        page->entries[i].value = 0;
    }
}


/*
 * Get physical address of a user virtual address
 */
ulong get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    int i;
    ulong paddr = 0;
    
    volatile struct page_frame *page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR(page_dir_pfn));
    int shift = PAGE_BITS + PAGE_ENTRY_BITS * (PAGE_LEVELS - 1);
    int index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
    int block_shift = PAGE_BITS;
    
    for (i = 0; i < PAGE_LEVELS - 1; i++) {
        // Check if mapping is valid
        if (!page->entries[index].value) {
            return 0;
        }
        
        // Move to next level
        if (page->entries[index].has_next_level) {
            page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR((ulong)page->entries[index].pfn));
            shift -= PAGE_ENTRY_BITS;
            index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
            block_shift += PAGE_ENTRY_BITS;
        }
        
        // Done at this level
        else {
            paddr = PFN_TO_ADDR((ulong)page->entries[index].pfn);
            paddr |= vaddr & ((0x1ul << block_shift) - 1);
            return paddr;
        }
    }
    
    // The final level
    assert(!page->entries[index].has_next_level);
    paddr = PFN_TO_ADDR((ulong)page->entries[index].pfn);
    paddr |= vaddr & (PAGE_SIZE - 1);
    
    return paddr;
}


/*
 * User memory mapping
 */
static int user_indirect_map(
    ulong page_dir_pfn, ulong vaddr, ulong paddr,
    int exec, int write, int cacheable, int override)
{
    int i;
    
    // Loop through page table levels to find the final level
    volatile struct page_frame *page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR(page_dir_pfn));
    int shift = PAGE_BITS + PAGE_ENTRY_BITS * (PAGE_LEVELS - 1);
    int index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
    
    for (i = 0; i < PAGE_LEVELS - 1; i++) {
        // Allocate a new page if necessary
        if (!page->entries[index].value) {
            ulong alloc_pfn = kernel->palloc(1);
            if (!alloc_pfn) {
                return 0;
            }
            
            // FIXME: the page should've been zeroed by kernel
            memzero((void *)PHYS_TO_KDATA(PFN_TO_ADDR(alloc_pfn)), PAGE_SIZE);

            page->entries[index].pfn = alloc_pfn;
            page->entries[index].present = 1;
            page->entries[index].has_next_level = 1;
        }
        
        // Doesn't support block mapping yet
        assert(page->entries[index].has_next_level);
        
        // Move to next level
        page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR((ulong)page->entries[index].pfn));
        shift -= PAGE_ENTRY_BITS;
        index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
    }
    
    // If the mapping had been created
    if (page->entries[index].value) {
        if (
            page->entries[index].pfn != ADDR_TO_PFN(paddr) ||
            !page->entries[index].present
        ) {
            assert(page->entries[index].pfn == ADDR_TO_PFN(paddr));
            assert(page->entries[index].present);
            
            return 0;
        }
        
        if (override) {
            page->entries[index].exec_allow = exec;
            page->entries[index].write_allow = write;
            page->entries[index].cache_allow = cacheable;
        } else  if (
            page->entries[index].exec_allow != exec ||
            page->entries[index].write_allow != write ||
            page->entries[index].cache_allow != cacheable
        ) {
            return 0;
        }
    }
    
    // This is our first time mapping the page
    else {
        page->entries[index].pfn = ADDR_TO_PFN(paddr);
        page->entries[index].present = 1;
        page->entries[index].has_next_level = 0;
        page->entries[index].exec_allow = exec;
        page->entries[index].write_allow = write;
        page->entries[index].cache_allow = cacheable;
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
    int i, j, level;
    
    // Loop through page table levels to find the final level
    volatile struct page_frame *page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR(page_dir_pfn));
    int shift = PAGE_BITS + PAGE_ENTRY_BITS * (PAGE_LEVELS - 1);
    int index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
    
    for (i = 0; i < PAGE_LEVELS - 1; i++) {
        // Doesn't support block mapping yet
        assert(page->entries[index].has_next_level);
        assert(page->entries[index].present);
        assert(page->entries[index].pfn);
        
        // Move to next level
        page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR((ulong)page->entries[index].pfn));
        shift -= PAGE_ENTRY_BITS;
        index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
    }
    
    // Unmap
    assert(page->entries[index].pfn == ADDR_TO_PFN(paddr));
    page->entries[index].value = 0;
    
    // Loop through page table again to see if intermediate page tables can be freed
    for (level = PAGE_LEVELS; level > 1; level--) {
        page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR(page_dir_pfn));
        shift = PAGE_BITS + PAGE_ENTRY_BITS * (PAGE_LEVELS - 1);
        index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
        
        for (i = 0; i < level - 1; i++) {
            // Doesn't support block mapping yet
            assert(page->entries[index].has_next_level);
            assert(page->entries[index].present);
            assert(page->entries[index].pfn);
            
            // Move to next level
            page = (struct page_frame *)PHYS_TO_KDATA(PFN_TO_ADDR((ulong)page->entries[index].pfn));
            shift -= PAGE_ENTRY_BITS;
            index = (vaddr >> shift) & (PAGE_ENTRY_COUNT - 1);
        }
        
        // Got the page, see if it's an all-zero page
        int need_free = 1;
        
        for (index = 0; index < PAGE_ENTRY_COUNT; index++) {
            if (page->entries[index].value) {
                need_free = 0;
                break;
            }
        }
        
        // Free the page
        if (need_free) {
            assert(kernel->pfree(ADDR_TO_PFN((ulong)page)));
        }
    }
    
    return 1;
}

int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length)
{
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
