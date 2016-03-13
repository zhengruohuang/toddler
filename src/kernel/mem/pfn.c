#include "common/include/data.h"
#include "kernel/include/hal.h"


struct pfndb_entry {
    union {
        u32 flags;
        
        struct {
            u32 usable      : 1;
            u32 mapped      : 1;
            u32 tag         : 4;
            u32 inuse       : 1;
            u32 zeroed      : 1;
            u32 kernel      : 1;
            u32 swappable   : 1;
        };
    };
} packedstruct;


//static struct pfndb_entry *pfndb;


void init_pfndb()
{
    kprintf("Initializing PFN database\n");
    
    // Calculate the size of PFN DB
}
