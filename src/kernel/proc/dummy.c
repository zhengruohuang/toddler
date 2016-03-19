/*
 * Dummy kernel threads
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"
#include "kernel/include/proc.h"


static spinlock_t demo_lock = { 0 };


void kernel_idle_thread(ulong param)
{
    do {
        kprintf("Dummy thread\n");
        hal->sleep();
    } while (1);
}

void kernel_demo_thread(ulong param)
{
    do {
        spin_lock_int(&demo_lock);
        int index = param;
        kprintf("This is kernel demo thread #%d on CPU #%d!\n", index, hal->get_cur_cpu_id());
        spin_unlock_int(&demo_lock);
        hal->sleep();
    } while (1);
}
