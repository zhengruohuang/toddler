/*
 * Kernel threads
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/sync.h"
#include "kernel/include/mem.h"
#include "kernel/include/syscall.h"
#include "kernel/include/proc.h"


asmlinkage void kernel_idle_thread(ulong param)
{
    do {
//         spin_lock_int(&dummy_thread_lock);
//         int index = param;
//         kprintf("This is kernel dummy thread #%d on CPU #%d!\n", index, hal->get_cur_cpu_id());
//         spin_unlock_int(&dummy_thread_lock);
//         hal->loop();
        while (1);
    } while (1);
}

asmlinkage void kernel_demo_thread(ulong param)
{
    do {
        int index = param;
        int cpu_id = hal->get_cur_cpu_id();
//         kprintf("Kernel demo thread #%d on CPU #%d%s!\n", index, cpu_id , index == cpu_id ? ", Thread ID == CPU ID" : "");
        //hal->yield();
        ksys_yield();
    } while (1);
}

asmlinkage void kernel_tclean_thread(ulong param)
{
    struct process *p = (struct process *)param;
    
    do {
//         kprintf("Cleaning thrads\n");
        destroy_absent_threads(p);
        //buddy_print();
        //hal->yield();
        ksys_yield();
    } while (1);
}
