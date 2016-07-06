/*
 * KAPI Handling - Thread
 */
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg)
{
    kprintf("To terminate user thread: %p\n", arg->sender_thread);
    
    terminate_thread(arg->sender_thread);
    terminate_thread(arg->handler_thread);
    
    sfree(arg);
    
    kprintf("Threads should have been terminated\n");
    
    // Wait for this thread to be terminated
    do {
        hal->sleep();
    } while (1);
    
    // Should never reach here
    kprintf("kapi.c: Should never reach here!\n");
    do {
        hal->sleep();
    } while (1);
}
