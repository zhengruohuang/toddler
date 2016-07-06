/*
 * KAPI Handling - Interrupt
 */
#include "common/include/syscall.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/syscall.h"
#include "kernel/include/kapi.h"


asmlinkage void reg_interrupt_handler(struct kernel_msg_handler_arg *arg)
{
    // Get the params
    struct process *p = arg->sender_thread->proc;
    ulong irq = arg->msg->params[0].value;
    ulong thread_entry = arg->msg->params[1].value;
    
    reg_interrupt(p, irq, thread_entry);
    
    run_thread(arg->sender_thread);
    terminate_thread(arg->handler_thread);
    sfree(arg);
    
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

asmlinkage void unreg_interrupt_handler(struct kernel_msg_handler_arg *arg)
{
    // Get the params
    struct process *p = arg->sender_thread->proc;
    ulong irq = arg->msg->params[0].value;
    
    unreg_interrupt(p, irq);
    
    run_thread(arg->sender_thread);
    terminate_thread(arg->handler_thread);
    sfree(arg);
    
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
