#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/bootparam.h"
#include "hal/include/bit.h"
#include "hal/include/string.h"
#include "hal/include/debug.h"
#include "hal/include/cpu.h"
#include "hal/include/kernel.h"
#include "hal/include/mem.h"


struct l1table *kernel_l1table;


/*
 * Initialize user page directory
 */
void init_user_page_dir(ulong page_dir_pfn)
{
    volatile struct l1table *l1tab = (struct l1table *)PFN_TO_ADDR(page_dir_pfn);

    int i;
    for (i = 0; i < 4096; i++) {
        l1tab->value_u32[i] = 0;
    }
    
    // Duplicate the last 4MB mapping
    l1tab->value_u32[4095] = kernel_l1table->value_u32[4095];
    l1tab->value_u32[4094] = kernel_l1table->value_u32[4094];
    l1tab->value_u32[4093] = kernel_l1table->value_u32[4093];
    l1tab->value_u32[4092] = kernel_l1table->value_u32[4092];
}


/*
 * Get physical address of a user virtual address
 */
ulong get_paddr(ulong page_dir_pfn, ulong vaddr)
{
    // L1 table
    volatile struct l1table *l1tab = (struct l1table *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_L1PTE_INDEX(vaddr);
    
    // Not mapped yet
//     assert(l1tab->value_u32[index]);
    if (!l1tab->value_u32[index]) {
        return 0;
    }
    
    // 1MB section
    if (l1tab->value_l1section[index].present) {
        ulong paddr = SFN_TO_ADDR((ulong)l1tab->value_l1section[index].sfn);
        paddr |= vaddr & 0xFFFFFul;
        return paddr;
    }
    
    // L2 table
    assert(l1tab->value_l1pte[index].present);
    volatile struct l2table *l2tab = (struct l2table *)(PFN_TO_ADDR((ulong)l1tab->value_l1pte[index].pfn));
    index = GET_L2PTE_INDEX(vaddr);
    
    // Not mapped yet
//     assert(l2tab->value_u32[index]);
    if (!l2tab->value_u32[index]) {
        return 0;
    }
    
    // Paddr
    ulong paddr = PFN_TO_ADDR((ulong)l2tab->value_l2pte[index].pfn);
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
//     disable_local_int();
//     panic("User indirect map!\n");
    
    int i;
    
    // L1 table
    volatile struct l1table *l1tab = (struct l1table *)PFN_TO_ADDR(page_dir_pfn);
    int index = GET_L1PTE_INDEX(vaddr);
    
    if (!l1tab->value_u32[index]) {
        ulong alloc_pfn = kernel->palloc(1);
        if (!alloc_pfn) {
            return 0;
        }
        
        // FIXME: the page should've been zeroed by kernel
        memzero((void *)PFN_TO_ADDR(alloc_pfn), PAGE_SIZE);
        
        l1tab->value_l1pte[index].present = 1;
        l1tab->value_l1pte[index].pfn = alloc_pfn;
    }
    
    assert(l1tab->value_l1pte[index].present);
    
    // L2 table
    volatile struct l2table *l2tab = (struct l2table *)(PFN_TO_ADDR((ulong)l1tab->value_l1pte[index].pfn));
    index = GET_L2PTE_INDEX(vaddr);
    
    if (l2tab->value_u32[index]) {
        if (
            l2tab->value_l2pte[index].pfn != ADDR_TO_PFN(paddr) ||
            !l2tab->value_l2pte[index].present
        ) {
            //kprintf("Old PFN: %p, new PFN: %p, present: %d, user: %d\n",
            //    page->value_pte[index].pfn, ADDR_TO_PFN(paddr), page->value_pde[index].present, page->value_pde[index].user);
            
            return 0;
        }
        
        if (override) {
            l2tab->value_l2pte[index].no_exec = !exec;
            l2tab->value_l2pte[index].user_write = write;
            l2tab->value_l2pte[index].user_access = 1;    // User read
            l2tab->value_l2pte[index].cache_inner = cacheable ? 0x3 : 0;  // Cacheable, write-back, write-allocate
            
            // Make dups
            for (i = 0; i < 3; i++) {
                l2tab->value_l2pte_dup[i][index].value = l2tab->value_l2pte[index].value;
            }
        } else if (
            l2tab->value_l2pte[index].no_exec != !exec ||
            l2tab->value_l2pte[index].user_write != write ||
            l2tab->value_l2pte[index].cache_inner != (cacheable ? 0x3 : 0)
        ) {
            return 0;
        }
    }
    
    // This is our first time mapping the page
    else {
        l2tab->value_l2pte[index].present = 1;
        l2tab->value_l2pte[index].pfn = ADDR_TO_PFN(paddr);
        l2tab->value_l2pte[index].no_exec = !exec;
        l2tab->value_l2pte[index].user_write = write;
        l2tab->value_l2pte[index].user_access = 1;    // User read
        l2tab->value_l2pte[index].cache_inner = cacheable ? 0x3 : 0;  // Cacheable, write-back, write-allocate
        
        // Make dups
        for (i = 0; i < 3; i++) {
            l2tab->value_l2pte_dup[i][index].value = l2tab->value_l2pte[index].value;
        }
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
//     panic("Doing unmap!\n");
//     disable_local_int();
//     while (1);
    
    int need_free = 1;
    
    // L1 table
    volatile struct l1table *l1tab = (struct l1table *)PFN_TO_ADDR(page_dir_pfn);
    int l1index = GET_L1PTE_INDEX(vaddr);
    assert(l1tab->value_u32[l1index]);
    
    // L2 table
    volatile struct l2table *l2tab = (struct l2table *)(PFN_TO_ADDR((ulong)l1tab->value_l1pte[l1index].pfn));
    int l2index = GET_L2PTE_INDEX(vaddr);
    assert(l2tab->value_u32[l2index]);
    
    // Unmap
    assert(l2tab->value_l2pte[l2index].pfn == ADDR_TO_PFN(paddr));
    l2tab->value_u32[l2index] = 0;
    
    // Free PTE page
    int i;
    for (i = 0; i < L2TABLE_ENTRY_COUNT; i++) {
        if (l2tab->value_u32[i]) {
            need_free = 0;
            break;
        }
    }
    
    if (need_free) {
        assert(kernel->pfree(ADDR_TO_PFN((ulong)l2tab)));
        
        // FIXME: this stmt should be added to other architectures !!!
        l1tab->value_u32[l1index] = 0;
    }
    
    return 1;
}

int user_indirect_unmap_array(ulong page_dir_pfn, ulong vaddr, ulong paddr, size_t length)
{
    //kprintf("To unmap, pfn: %u, vaddr: %u, paddr: %u, size: %u\n", page_dir_pfn, vaddr, paddr, length);
    
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
    
    int i, j;
    
    for (; vaddr < vend; vaddr += PAGE_SIZE, paddr += PAGE_SIZE) {
        int l1index = GET_L1PTE_INDEX(vaddr);
        volatile struct l2table *l2tab = NULL;
        
//         kprintf("Mapping vaddr @ %lx, paddr @ %lx\n", vaddr, paddr);
//         kprintf("L1table index: %d\n", l1index);
        
        if (kernel_l1table->value_l1pte[l1index].present) {
            assert(!kernel_l1table->value_l1section[l1index].present);
            l2tab = (struct l2table *)(PFN_TO_ADDR((ulong)kernel_l1table->value_l1pte[l1index].pfn));
        }
        
        else {
            assert(kernel_l1table->value_l1section[l1index].present);
            ulong sec_pfn = ADDR_TO_PFN(SFN_TO_ADDR((ulong)kernel_l1table->value_l1section[l1index].sfn));
            
//             kprintf("sec seq @ %x\n", kernel_l1table->value_l1section[l1index].sfn);
//             kprintf("sec pfn @ %lx\n", sec_pfn);
            
            // Allocate a new page for the L2 table
            ulong l2tab_pfn = palloc(1);
            l2tab = (struct l2table *)(PFN_TO_ADDR(l2tab_pfn));
            
//             kprintf("L2Tab allocated @ PFN %lx, @ %p\n", l2tab_pfn, l2tab);
            
            kernel_l1table->value_l1pte[l1index].value = 0;
            kernel_l1table->value_l1pte[l1index].present = 1;
            kernel_l1table->value_l1pte[l1index].pfn = (u32)l2tab_pfn;
            
            // Set up the initial 1 to 1 mapping
            for (i = 0; i < L2TABLE_ENTRY_COUNT; i++, sec_pfn++) {
                l2tab->value_l2pte[i].present = 1;
//                 l2tab->value_l2pte[i].no_exec = 1;
                l2tab->value_l2pte[i].user_write = 1;     // AP[1:0] = 01
                l2tab->value_l2pte[i].user_access = 0;    // Kernel RW, user no access
                l2tab->value_l2pte[i].cache_inner = 0x3;  // Cacheable, write-back, write-allocate
                l2tab->value_l2pte[i].pfn = (u32)sec_pfn;
            }
            
            for (i = 0; i < L2TABLE_ENTRY_COUNT; i++) {
                for (j = 0; j < 3; j++) {
                    l2tab->value_l2pte_dup[j][i].value = l2tab->value_l2pte[i].value;
                }
            }
        }
        
        int l2index = GET_L2PTE_INDEX(vaddr);
        assert(l2tab->value_l2pte[l2index].present);
        l2tab->value_l2pte[l2index].pfn = (u32)ADDR_TO_PFN(paddr);
        
//         kprintf("L2 table @ %p, index: %d, pfn: %x\n", l2tab, l2index, l2tab->value_l2pte[l2index].pfn);
        
        for (j = 0; j < 3; j++) {
            l2tab->value_l2pte_dup[j][l2index].value = l2tab->value_l2pte[l2index].value;
        }
    }
}


/*
 * Init mapping
 */
void init_map()
{
    struct boot_parameters *bp = get_bootparam();
    
    kernel_l1table = (struct l1table *)bp->hal_page_dir;
}
