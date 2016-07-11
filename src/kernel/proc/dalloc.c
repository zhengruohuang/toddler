/*
 * Thread dynamic area allocator
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/sync.h"
#include "kernel/include/proc.h"


static int dalloc_salloc_id = -1;


void init_dalloc()
{
    kprintf("Initializing dynamic memory allocator\n");
    
    // Create salloc obj
    dalloc_salloc_id = salloc_create(sizeof(struct dynamic_block), 0, 0, NULL, NULL);
}


void create_dalloc(struct process *p)
{
    if (p->type == process_kernel) {
        p->dynamic.cur_top = 0;
    } else {
        p->dynamic.cur_top = 0xf0000000;
        
        p->dynamic.free.count = 0;
        p->dynamic.free.head = NULL;
        spin_init(&p->dynamic.free.lock);
        
        p->dynamic.in_use.count = 0;
        p->dynamic.in_use.head = NULL;
        spin_init(&p->dynamic.in_use.lock);
    }
}


static void record_block(struct process *p, ulong base, ulong size)
{
    struct dynamic_block *block = (struct dynamic_block *)salloc(dalloc_salloc_id);
    assert(block);
    
    block->base = base;
    block->size = size;
    
    spin_lock_int(&p->dynamic.in_use.lock);
    
    block->next = p->dynamic.in_use.head;
    p->dynamic.in_use.head = block;
    
    p->dynamic.in_use.count ++;
    
    spin_unlock_int(&p->dynamic.in_use.lock);
}

static ulong alloc_new(struct process *p, ulong size)
{
    assert(p->dynamic.cur_top - p->memory.heap_end > size);
    
    p->dynamic.cur_top -= size;
    record_block(p, p->dynamic.cur_top, size);
    
    return p->dynamic.cur_top;
}

static ulong alloc_exist(struct process *p, ulong size)
{
    struct dynamic_block *prev, *cur;
    ulong result = 0;
    
    if (!p->dynamic.free.count) {
        return 0;
    }
    
    spin_lock_int(&p->dynamic.free.lock);
    
    prev = NULL;
    cur = p->dynamic.free.head;
    
    while (cur) {
        kprintf("here!\n");
        
        if (cur->size < size) {
            prev = cur;
            cur = cur->next;
            
            continue;
        }
        
        result = cur->base;
            
        if (cur->size == size) {
            if (prev) {
                prev->next = cur->next;
            } else {
                p->dynamic.free.head = NULL;
            }
            
            sfree(cur);
            p->dynamic.free.count--;
        } else {
            cur->base += size;
            cur->size -= size;
        }
        
        break;
    }
    
    spin_unlock_int(&p->dynamic.free.lock);
    
    if (result) {
        record_block(p, result, size);
    }
    
    kprintf("[DALLOC] Alloc from exist: %x\n", result);
    return result;
}

ulong dalloc(struct process *p, ulong size)
{
    assert(p->type != process_kernel);
    
    if (size % PAGE_SIZE) {
        size /= PAGE_SIZE;
        size++;
        size *= PAGE_SIZE;
    }
    
    ulong result = alloc_exist(p, size);
    if (!result) {
        result = alloc_new(p, size);
    }
    
    assert(result);
    return result;
}

static int combine_free_blocks(struct process *p)
{
    return 0;
}

void dfree(struct process *p, ulong base)
{
    int found = 0;
    struct dynamic_block *prev, *cur, *block;
    
    // Remove from in use
    spin_lock_int(&p->dynamic.in_use.lock);
    
    prev = NULL;
    cur = p->dynamic.in_use.head;
    
    while (cur) {
        if (cur->base == base) {
            found = 1;
            break;
        }
        
        prev = cur;
        cur = cur->next;
    }
    
    assert(found);
    block = cur;
    
    kprintf("[DALLOC] Found!\n");
    
    if (prev) {
        prev->next = cur->next;
    } else {
        p->dynamic.in_use.head = cur->next;
    }
    
    p->dynamic.in_use.count--;
    
    spin_unlock_int(&p->dynamic.in_use.lock);
    
    // Insert the block into free list
    spin_lock_int(&p->dynamic.free.lock);
    
    prev = NULL;
    cur = p->dynamic.free.head;
    
    while (cur) {
        if (cur->base < block->base) {
            break;
        }
        
        prev = cur;
        cur = cur->next;
    }
    
    block->next = cur;
    if (prev) {
        prev->next = block;
    } else {
        p->dynamic.free.head = block;
        kprintf("[DALLOC] Free head set!\n");
    }
    
    p->dynamic.free.count++;
    
    spin_unlock_int(&p->dynamic.free.lock);
    
    // Combine combine blocks
    while (combine_free_blocks(p));
}
