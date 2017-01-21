/*
 * KMap - Map kernel memory to user process
 */

#include "common/include/data.h"
#include "common/include/proc.h"
#include "common/include/coreimg.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


static unsigned long do_kmap_coreimg(struct process *p)
{
    // Get the image location and size
    unsigned long coreimg_addr = hal->coreimg_load_addr;
    struct coreimg_header *header = (struct coreimg_header *)hal->coreimg_load_addr;
    unsigned long size = header->image_size;
    
    // Allocate a block in virtual address space
    unsigned long user_vaddr = dalloc(p, size);
    assert(user_vaddr);
    
    // Map the image to the block
    int succeed = hal->map_user(
        p->page_dir_pfn,
        user_vaddr, coreimg_addr, size,
        0, 0, 1, 0
    );
    assert(succeed);
    
    // Done
    return user_vaddr;
}

unsigned long kmap(struct process *p, enum kmap_region region)
{
    // First of all check if the user process has the permission
    
    // Do the mapping
    switch (region) {
    case kmap_coreimg:
        return do_kmap_coreimg(p);
    case kmap_none:
    default:
        return 0;
    }
    
    return 0;
}
