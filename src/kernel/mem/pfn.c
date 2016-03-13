#include "common/include/data.h"
#include "kernel/include/hal.h"


struct pfndb_entry {
    union {
        u16 flags;
        
        struct {
            u16 usable      : 1;
            u16 mapped      : 1;
            u16 tag         : 4;
            u16 inuse       : 1;
            u16 zeroed      : 1;
            u16 kernel      : 1;
            u16 swappable   : 1;
        };
    };
} packedstruct;


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

void init_pfndb()
{
    kprintf("Initializing PFN database\n");
    
    // Calculate the size of PFN DB
    int pfndb_size = hal->paddr_space_end / PAGE_SIZE * sizeof(struct pfndb_entry);
    int pfndb_pages = pfndb_size / PAGE_SIZE;
    kprintf("\tPFN database size: %d KB, pages: %d\n", pfndb_size / 1024, pfndb_pages);
    
    // Allocate memory
    pfndb = (struct pfndb_entry *)hal->free_mem_start_addr;
    hal->free_mem_start_addr += pfndb_size;
    
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
                kprintf(".");
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
    }
    
    kprintf("\n");
}
