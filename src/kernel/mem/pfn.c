#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"


static struct pfndb_entry *pfndb;


struct pfndb_entry *get_pfn_entry_by_pfn(ulong pfn)
{
    return &pfndb[pfn];
}

struct pfndb_entry *get_pfn_entry_by_paddr(ulong paddr)
{
    ulong pfn = ADDR_TO_PFN(paddr);
    return &pfndb[pfn];
}

void reserve_pfndb_mem(ulong start, ulong size)
{
    ulong i;
    ulong end = start + size;
    
    kprintf("\tReserving memory @ %x to %x ...", start, end);
    
    for (i = start; i < end; i += PAGE_SIZE) {
        struct pfndb_entry *entry = get_pfn_entry_by_paddr(i);
        
        entry->inuse = 1;
        entry->zeroed = 0;
        entry->kernel = 1;
        entry->swappable = 0;
        entry->tag = 9;
        
        kprintf(".");
    }
    
    kprintf(" done\n");
}

void init_pfndb()
{
    kprintf("Initializing PFN database\n");
    
    // Calculate the size of PFN DB
    int pfndb_size = hal->paddr_space_end / PAGE_SIZE * sizeof(struct pfndb_entry);
    int pfndb_pages = pfndb_size / PAGE_SIZE;
    kprintf("\tPAddr end: %p, PFN database size: %d KB, pages: %d\n", hal->paddr_space_end, pfndb_size / 1024, pfndb_pages);
    
    // Allocate memory
    pfndb = (struct pfndb_entry *)hal->free_mem_start_addr;
    
    // Init the PFN DB
    kprintf("\tConstructing PFN database ...");
    
    struct kernel_mem_zone cur;
    cur.start = 0;
    cur.len = 0;
    
    struct pfndb_entry *entry;
    ulong prev_end = 0;
    ulong i;
    ulong count = 0;
    ulong total_entries = hal->paddr_space_end / PAGE_SIZE;
    
    while (hal->get_next_mem_zone(&cur)) {
        // Fill in the hole between two zones (is there is a hole)
        for (i = prev_end; i < cur.start; i += PAGE_SIZE) {
            entry = get_pfn_entry_by_paddr(i);
            
            entry->usable = 0;
            entry->mapped = 0;
            entry->tag = -1;
            entry->inuse = 1;
            entry->zeroed = 0;
            entry->kernel = 1;
            entry->swappable = 0;
            
            // Show progress
            if (0 == count++ % (total_entries / 10)) {
                kprintf("_");
            }
        }
        
        // Initialize the actual PFN entries
        for (i = cur.start; i < cur.start + cur.len; i += PAGE_SIZE) {
            entry = get_pfn_entry_by_paddr(i);
            
            entry->usable = cur.usable;
            entry->mapped = cur.mapped;
            entry->tag = cur.tag;
            entry->inuse = cur.inuse;
            entry->zeroed = 0;
            entry->kernel = cur.kernel;
            entry->swappable = cur.swappable;
            
            // Show progress
            if (0 == count++ % (total_entries / 10)) {
                kprintf(".");
            }
        }
        
        prev_end = cur.start + cur.len;
    }
    
    kprintf("\n\tRemaining: %p, End: %p ...", prev_end, hal->paddr_space_end);
    
    // Fill the rest of the PFN database
    for (i = prev_end; i < hal->paddr_space_end; i += PAGE_SIZE) {
        entry = get_pfn_entry_by_paddr(i);
        
        entry->usable = 0;
        entry->mapped = 0;
        entry->tag = -1;
        entry->inuse = 1;
        entry->zeroed = 0;
        entry->kernel = 1;
        entry->swappable = 0;
        
        kprintf(".");
    }
    
    kprintf("\n");
    
    // Mark PFN database memory as inuse
    reserve_pfndb_mem(hal->free_mem_start_addr, pfndb_size);
    hal->free_mem_start_addr += pfndb_size;
}
