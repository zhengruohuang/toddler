/*
 * Kernel threads
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"
#include "kernel/include/proc.h"


void kernel_idle_thread(ulong param)
{
    do {
//         spin_lock_int(&dummy_thread_lock);
//         int index = param;
//         kprintf("This is kernel dummy thread #%d on CPU #%d!\n", index, hal->get_cur_cpu_id());
//         spin_unlock_int(&dummy_thread_lock);
        hal->sleep();
    } while (1);
}

void kernel_demo_thread(ulong param)
{
    do {
        int index = param;
        int cpu_id = hal->get_cur_cpu_id();
        kprintf("Kernel demo thread #%d on CPU #%d%s!\n", index, cpu_id , index == cpu_id ? ", Thread ID == CPU ID" : "");
        hal->sleep();
    } while (1);
}

void kernel_tclean_thread(ulong param)
{
    do {
        int index = param;
        int cpu_id = hal->get_cur_cpu_id();
        kprintf("Kernel demo thread #%d on CPU #%d%s!\n", index, cpu_id , index == cpu_id ? ", Thread ID == CPU ID" : "");
        hal->sleep();
    } while (1);
}
