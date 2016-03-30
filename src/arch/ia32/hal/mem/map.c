#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/lib.h"
#include "hal/include/kernel.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"


static ulong user_hi4_pfn;

/*
 * User mapping
 */
void init_user_hi4()
{
    // Allocate a page
    user_hi4_pfn = palloc(1);
    assert(user_hi4_pfn);
    
    // Obtain the pages
    struct page_frame *kpage = (struct page_frame *)PFN_TO_ADDR(KERNEL_PTE_HI4_PFN);
    struct page_frame *upage = (struct page_frame *)PFN_TO_ADDR(user_hi4_pfn);
    
    // Duplicate the content
    int i;
    for (i = 0; i < 1024; i++) {
        upage->value_u32[i] = kpage->value_u32[i];
    }
    
    // Setup user version specific content
    upage->value_pte[GET_PTE_INDEX(SYSCALL_PROXY_VADDR)].user = 1;
    upage->value_pte[GET_PTE_INDEX(SYSCALL_PROXY_VADDR)].rw = 0;
    
    for (i = 0; i < num_cpus; i++) {
        ulong tcb_start = get_per_cpu_tcb_start_vaddr(i);
        
        upage->value_pte[GET_PTE_INDEX(tcb_start)].user = 1;
        upage->value_pte[GET_PTE_INDEX(tcb_start)].rw = 0;
    }
}

void init_user_page_dir(ulong page_dir_pfn)
{
    struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);

    int i;
    for (i = 0; i < 1024; i++) {
        page->value_u32[i] = 0;
    }
    
    page->value_pde[1023].pfn = user_hi4_pfn/*KERNEL_PTE_HI4_PFN*/;
    page->value_pde[1023].present = 1;
    page->value_pde[1023].rw = 1;
    page->value_pde[1023].user = 1;
    page->value_pde[1023].cache_disabled = 0;
    
//     page->value_pde[0].pfn = KERNEL_PTE_LO4_PFN;
//     page->value_pde[0].present = 1;
//     page->value_pde[0].rw = 1;
//     page->value_pde[0].user = 0;
//     page->value_pde[0].cache_disabled = 0;
}

ulong get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    // PDE
    struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
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

static int user_indirect_map(
    ulong page_dir_pfn, ulong vaddr, ulong paddr,
    int exec, int write, int cacheable, int override)
{
    // PDE
    struct page_frame *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
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
        page->value_pde[index].rw = 1;
        page->value_pde[index].user = 1;
    }
    
    if (!page->value_pde[index].present || !page->value_pde[index].user) {
        return 0;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (page->value_u32[index]) {
        if (
            page->value_pte[index].pfn != ADDR_TO_PFN(paddr) ||
            !page->value_pde[index].present ||
            !page->value_pde[index].user
        ) {
            //kprintf("Old PFN: %p, new PFN: %p, present: %d, user: %d\n", page->value_pte[index].pfn, ADDR_TO_PFN(paddr), page->value_pde[index].present, page->value_pde[index].user);
            
            return 0;
        }
        
        if (override) {
            page->value_pde[index].rw = write;
            page->value_pde[index].cache_disabled = !cacheable;
        } else  if (
            page->value_pde[index].rw != write ||
            page->value_pde[index].cache_disabled != !cacheable
        ) {
            return 0;
        }
    }
    
    // This is our first time mapping the page
    else {
        page->value_pte[index].pfn = ADDR_TO_PFN(paddr);
        page->value_pde[index].present = 1;
        page->value_pde[index].user = 1;
        page->value_pde[index].rw = write;
        page->value_pde[index].cache_disabled = !cacheable;
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


/*
 * Kernel mapping
 */
void kernel_indirect_map(ulong vaddr, ulong paddr, int disable_cache, int override)
{
    // PDE
    struct page_frame *page = (struct page_frame *)KERNEL_PDE_PADDR;
    int index = GET_PDE_INDEX(vaddr);
    
    if (!page->value_u32[index]) {
        page->value_pde[index].pfn = palloc(1);
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
    }
    
    // PTE
    page = (struct page_frame *)(PFN_TO_ADDR(page->value_pde[index].pfn));
    index = GET_PTE_INDEX(vaddr);
    
    if (page->value_u32[index] && !override) {
        if (page->value_pte[index].pfn != ADDR_TO_PFN(paddr)) {
            kprintf("Inconsistency detected, original PFN: %p, new PFN: %p\n",
                    page->value_pte[index].pfn, ADDR_TO_PFN(paddr));
        }
        
        assert(
            page->value_pte[index].pfn == ADDR_TO_PFN(paddr) &&
            page->value_pde[index].present &&
            page->value_pde[index].rw
        );
        
        if (disable_cache) {
            page->value_pde[index].cache_disabled = 1;
        }
    } else {
        page->value_pte[index].pfn = ADDR_TO_PFN(paddr);
        page->value_pde[index].present = 1;
        page->value_pde[index].rw = 1;
        page->value_pde[index].cache_disabled = disable_cache;
    }
}

void kernel_indirect_map_array(ulong vaddr, ulong paddr, size_t size, int disable_cache, int override)
{
    assert(vaddr % PAGE_SIZE == paddr % PAGE_SIZE);
    
    // Round down start addr
    ulong vstart = (vaddr / PAGE_SIZE) * PAGE_SIZE;
    ulong pstart = (paddr / PAGE_SIZE) * PAGE_SIZE;
    
    // Round up end addr
    ulong pend = paddr + size;
    if (pend % PAGE_SIZE) {
        pend /= PAGE_SIZE;
        pend++;
        pend *= PAGE_SIZE;
    }
    
    // Calculate the real size and page count
    ulong real_size = pend - pstart;
    ulong page_count = real_size / PAGE_SIZE;
    if (real_size % PAGE_SIZE) {
        page_count++;
    }
    
    ulong i;
    ulong vcur = vstart;
    ulong pcur = pstart;
    for (i = 0; i < page_count; i++) {
        kernel_indirect_map(vcur, pcur, disable_cache, override);
        
        vcur += PAGE_SIZE;
        pcur += PAGE_SIZE;
    }
}

void kernel_direct_map(ulong addr, int disable_cache)
{
    kernel_indirect_map(addr, addr, disable_cache, 0);
}

void kernel_direct_map_array(ulong addr, size_t size, int disable_cache)
{
    // Round down start addr
    ulong start = addr / PAGE_SIZE * PAGE_SIZE;
    
    // Round up end addr
    ulong end = addr + size;
    if (end % PAGE_SIZE) {
        end /= PAGE_SIZE;
        end++;
        end *= PAGE_SIZE;
    }
    
    // Calculate the real size and page count
    ulong real_size = end - start;
    ulong page_count = real_size / PAGE_SIZE;
    if (real_size % PAGE_SIZE) {
        page_count++;
    }
    
    ulong cur = start;
    ulong i;
    for (i = 0; i < page_count; i++) {
        kernel_direct_map(cur, disable_cache);
        cur += PAGE_SIZE;
    }
}
