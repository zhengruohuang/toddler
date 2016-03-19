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
        kprintf("Dummy thread\n");
        hal->sleep();
    } while (1);
}

void kernel_demo_thread(ulong param)
{
    do {
        __asm__ __volatile__
        (
            "cli;"
            :
            :
        );
        
        kprintf("This is kernel demo thread!\n");
        
        __asm__ __volatile__
        (
            "sti;"
            :
            :
        );
    } while (1);
}
