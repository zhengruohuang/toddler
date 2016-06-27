/*
 * Thread dynamic area allocator
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


void create_dalloc(struct process *p)
{
    if (p->type == process_kernel) {
        p->dynamic.cur_top = 0;
    } else {
        p->dynamic.cur_top = 0xf0000000;
    }
}

ulong dalloc(struct process *p, ulong size)
{
    if (size % PAGE_SIZE) {
        size /= PAGE_SIZE;
        size++;
        size *= PAGE_SIZE;
    }
    
    assert(p->dynamic.cur_top - p->memory.heap_end > size);
    assert(p->type != process_kernel);
    
    p->dynamic.cur_top -= size;
    
    return p->dynamic.cur_top;
}

void dfree(struct process *p, ulong base)
{
}
