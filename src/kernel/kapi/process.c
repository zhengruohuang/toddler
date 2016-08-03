/*
 * KAPI Handling - Process
 */
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"
#include "kernel/include/coreimg.h"


/*
 * Process started signal
 */
asmlinkage void process_started_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    startup_process_started(t->proc_id);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}


/*
 * Process control
 */
asmlinkage void process_exit_handler(struct kernel_msg_handler_arg *arg)
{

}
