/*
 * Kernel ASID manager
 */


#include "common/include/data.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"
#include "kernel/include/proc.h"


static spinlock_t lock;
static ulong cur_asid = 1;


void init_asid()
{
    spin_init(&lock);
}

void asid_release()
{
}

ulong asid_alloc()
{
    ulong asid = 0;
    
    spin_lock_int(&lock);
    
    asid = cur_asid;
    cur_asid++;
    
    atomic_membar();
    spin_unlock_int(&lock);
    
    return asid;
}

ulong asid_recycle()
{
    return 0;
}
