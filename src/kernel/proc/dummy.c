/*
 * Dummy kernel threads
 */


#include "common/include/data.h"
#include "common/include/memory.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


void kernel_idle_thread(ulong param)
{
    do {
        //kprintf("This is kernel idle thread!\n");
        //hal->sleep();
    } while (1);
}

void kernel_demo_thread(ulong param)
{
    do {
        //kprintf("This is kernel demo thread!\n");
        //hal->sleep();
    } while (1);
}
