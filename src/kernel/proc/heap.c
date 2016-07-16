/*
 * Process heap manager
 */

#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static void do_grow_heap(struct process *p, ulong amount)
{
    ulong old_vpfn = ADDR_TO_PFN(p->memory.heap_end);
    ulong new_vpfn = ADDR_TO_PFN(p->memory.heap_end + amount);
    
    ulong cur_vpfn;
    ulong cur_vaddr;
    ulong cur_paddr;
    
    if (new_vpfn <= old_vpfn) {
        return;
    }
    
    kprintf("old_vpfn: %p, new_vpfn: %p\n", old_vpfn, new_vpfn);
    
    for (cur_vpfn = old_vpfn + 1; cur_vpfn <= new_vpfn; cur_vpfn++) {
        cur_vaddr = PFN_TO_ADDR(cur_vpfn);
        cur_paddr = PFN_TO_ADDR(palloc(1));
        assert(cur_paddr);
        
        int succeed = hal->map_user(
            p->page_dir_pfn,
            cur_vaddr, cur_paddr, PAGE_SIZE,
            0, 1, 1, 0
        );
        assert(succeed);
    }
    
    p->memory.heap_end += amount;
}

static void do_shrink_heap(struct process *p, ulong amount)
{
    ulong old_vpfn = ADDR_TO_PFN(p->memory.heap_end);
    ulong new_vpfn = ADDR_TO_PFN(p->memory.heap_end - amount);
    
    ulong cur_vpfn;
    ulong cur_vaddr;
    ulong cur_paddr;
    
    if (new_vpfn >= old_vpfn) {
        return;
    }
    
    for (cur_vpfn = old_vpfn; cur_vpfn > new_vpfn; cur_vpfn--) {
        cur_vaddr = PFN_TO_ADDR(cur_vpfn);
        cur_paddr = hal->get_paddr(p->page_dir_pfn, cur_vaddr);
        assert(cur_paddr);
        
        int succeed = hal->unmap_user(
            p->page_dir_pfn,
            cur_vaddr, cur_paddr, PAGE_SIZE
        );
        assert(succeed);
    }
    
    // TLB shootdown
    trigger_tlb_shootdown(PFN_TO_ADDR((new_vpfn + 1)), PAGE_SIZE * (int)(old_vpfn - new_vpfn));
    
    p->memory.heap_end -= amount;
}

ulong set_heap_end(struct process *p, ulong heap_end)
{
    spin_lock_int(&p->lock);
    
    if (heap_end < p->memory.heap_start || heap_end > p->memory.dynamic_bottom) {
        spin_unlock_int(&p->lock);
        return 0;
    }
    
    if (heap_end == p->memory.heap_end) {
        spin_unlock_int(&p->lock);
        return heap_end;
    }
    
    // Grow & Shrink
    else if (heap_end > p->memory.heap_end) {
        do_grow_heap(p, heap_end - p->memory.heap_end);
    } else if (heap_end < p->memory.heap_end) {
        do_shrink_heap(p, p->memory.heap_end - heap_end);
    }
    
    spin_unlock_int(&p->lock);
    
    return heap_end;
}

ulong get_heap_end(struct process *p)
{
    ulong result = 0;
    
    spin_lock_int(&p->lock);
    
    result = p->memory.heap_end;
    
    spin_unlock_int(&p->lock);
    
    return result;
}

ulong grow_heap(struct process *p, ulong amount)
{
    spin_lock_int(&p->lock);
    
    if (p->memory.dynamic_bottom - p->memory.heap_end < amount) {
        spin_unlock_int(&p->lock);
        return 0;
    }
    
    do_grow_heap(p, amount);
    
    spin_unlock_int(&p->lock);
    
    return amount;
}

ulong shrink_heap(struct process *p, ulong amount)
{
    if (!amount) {
        return 0;
    }
    
    spin_lock_int(&p->lock);
    
    if (p->memory.heap_end - p->memory.heap_start < amount) {
        spin_unlock_int(&p->lock);
        return 0;
    }
    
    do_shrink_heap(p, amount);
    
    spin_unlock_int(&p->lock);
    
    return amount;
}
