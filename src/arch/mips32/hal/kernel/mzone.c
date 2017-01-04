#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/kernel.h"

#ifndef __HAL__
#define __HAL__
#endif
#include "kernel/include/hal.h"


#define MAX_KERNEL_MEM_ZONE_COUNT   32


static int mem_zone_count = 0;
static struct kernel_mem_zone mem_zones[MAX_KERNEL_MEM_ZONE_COUNT];

ulong paddr_space_end = 0x200000;


int get_next_mem_zone(struct kernel_mem_zone *cur)
{
    int i;
    
    if (!cur) {
        return 0;
    }
    
    for (i = 0; i < mem_zone_count; i++) {
        if (
            mem_zones[i].start > cur->start ||
            (0 == cur->start && 0 == cur->len)
        ) {
            cur->start = mem_zones[i].start;
            cur->len = mem_zones[i].len;
            cur->usable = mem_zones[i].usable;
            cur->mapped = mem_zones[i].mapped;
            cur->tag = mem_zones[i].tag;
            cur->inuse = mem_zones[i].inuse;
            
            return 1;
        }
    }
    
    return 0;
}

void init_kmem_zone()
{
    int i;
    
    kprintf("Initializing memory zone map\n");
    
    /*
     * Echo BIOS detected usable zones
     */
    kprintf("\tBIOS detected usable zones\n");
    for (i = 0; i < get_bootparam()->mem_zone_count; i++) {
        struct boot_mem_zone *cur_zone = &get_bootparam()->mem_zones[i];
        
        u32 start = (u32)cur_zone->start_paddr;
        u32 len = (u32)cur_zone->len;
        u32 end = start + len;
        kprintf("\t\tZone #%d, Start: %x, Len: %x, End: %x, Type: %d\n", i, start, len, end, cur_zone->type);
    }
    
    /*
     * Generate PFN DB zones
     */
    // Zone 0: Low 1MB, unusable, mapped, inuse
    mem_zones[0].start = 0;
    mem_zones[0].len = 0x100000;
    mem_zones[0].usable = 0;
    mem_zones[0].mapped = 1;
    mem_zones[0].tag = -1;
    mem_zones[0].inuse = 1;
    mem_zones[0].kernel = 1;
    mem_zones[0].swappable = 0;
    mem_zone_count++;
    
    // Zone 1: 1MB ~ 2MB: usable, mapped, inuse
    mem_zones[1].start = 0x100000;
    mem_zones[1].len = 0x100000;
    mem_zones[1].usable = 1;
    mem_zones[1].mapped = 1;
    mem_zones[1].tag = 0;
    mem_zones[1].inuse = 1;
    mem_zones[0].kernel = 1;
    mem_zones[0].swappable = 0;
    mem_zone_count++;
    
    // Other zones: go through BIOS mem zones, usable, unmapped
    for (i = 0; i < get_bootparam()->mem_zone_count; i++) {
        struct boot_mem_zone *cur_zone = &get_bootparam()->mem_zones[i];
        u32 start = (u32)cur_zone->start_paddr;
        u32 len = (u32)cur_zone->len;
        
        // Below 4MB: skip it
        if (start + len <= 0x200000) {
            continue;
        }
        
        // This zone goes across 4MB: split it
        else if (start < 0x200000 && start + len > 0x200000) {
            len = start + len - 0x200000;
            start = 0x200000;
        }
        
        // This zone is above 4MB
        else {
            // simply use it, don't do anything
        }
        
        // Construct the zone structure
        mem_zones[mem_zone_count].start = start;
        mem_zones[mem_zone_count].len = len;
        mem_zones[mem_zone_count].usable = 1;
        mem_zones[mem_zone_count].mapped = 1;
        mem_zones[mem_zone_count].tag = 0;
        mem_zones[mem_zone_count].inuse = 0;
        mem_zones[mem_zone_count].kernel = 0;
        mem_zones[mem_zone_count].swappable = 1;
        mem_zone_count++;
        
        // Update physical address space boundary
        paddr_space_end = start + len;
    }
    
    // Round up the physical address space boundary
    if (paddr_space_end % PAGE_SIZE) {
        paddr_space_end /= PAGE_SIZE;
        paddr_space_end++;
        paddr_space_end *= PAGE_SIZE;
    }
    
    /*
     * Echo final mem zone map
     */
    kprintf("\tPAddr space boundary: %p\n", paddr_space_end);
    kprintf("\tKernel memory zones\n");
    for (i = 0; i < mem_zone_count; i++) {
        u32 start = (u32)mem_zones[i].start;
        u32 len = (u32)mem_zones[i].len;
        u32 end = start + len;
        
        kprintf("\t\tZone #%d, Start: %x, Len: %x, End: %x, Usable: %d, Mapped: %d, Tag: %d, Inuse: %d\n",
            i, start, len, end, mem_zones[i].usable, mem_zones[i].mapped, mem_zones[i].tag, mem_zones[i].inuse
        );
    }
}
