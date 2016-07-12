/*
 * Process heap manager
 */

#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static void do_grow_heap(struct process *p, ulong amount)
{
    int i;
    int page_count = amount / PAGE_SIZE;
    
    ulong cur_vaddr = p->memory.heap_end;
    ulong cur_paddr = 0;
    
    for (i = 0; i < page_count; i++, cur_vaddr += PAGE_SIZE) {
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
    int i;
    int page_count = amount / PAGE_SIZE;
    
    ulong cur_vaddr = p->memory.heap_end - PAGE_SIZE;
    ulong cur_paddr = 0;
    
    for (i = 0; i < page_count; i++, cur_vaddr -= PAGE_SIZE) {
        cur_paddr = hal->get_paddr(p->page_dir_pfn, cur_vaddr);
        assert(cur_paddr);
        
        int succeed = hal->unmap_user(
            p->page_dir_pfn,
            cur_vaddr, cur_paddr, PAGE_SIZE
        );
        assert(succeed);
    }
    
    p->memory.heap_end -= amount;
}

ulong set_heap_end(struct process *p, ulong heap_end)
{
    if (heap_end % PAGE_SIZE) {
        heap_end /= PAGE_SIZE;
        heap_end++;
        heap_end *= PAGE_SIZE;
    }
    
    spin_lock_int(&p->lock);
    
    assert(p->memory.heap_end % PAGE_SIZE == 0);
    
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
    
    assert(p->memory.heap_end % PAGE_SIZE == 0);
    result = p->memory.heap_end;
    
    spin_unlock_int(&p->lock);
    
    return result;
}

ulong grow_heap(struct process *p, ulong amount)
{
    if (amount % PAGE_SIZE) {
        amount /= PAGE_SIZE;
        amount++;
        amount *= PAGE_SIZE;
    }
    
    spin_lock_int(&p->lock);
    
    assert(p->memory.heap_end % PAGE_SIZE == 0);
    
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
    if (amount % PAGE_SIZE) {
        amount /= PAGE_SIZE;
        amount--;
        amount *= PAGE_SIZE;
    }
    
    if (!amount) {
        return 0;
    }
    
    spin_lock_int(&p->lock);
    
    assert(p->memory.heap_end % PAGE_SIZE == 0);
    
    if (p->memory.heap_end - p->memory.heap_start < amount) {
        spin_unlock_int(&p->lock);
        return 0;
    }
    
    do_shrink_heap(p, amount);
    
    spin_unlock_int(&p->lock);
    
    return amount;
}
