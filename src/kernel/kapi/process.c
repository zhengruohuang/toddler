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
    
    // Resume sender thread
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


/*
 * Process event monitor
 */
asmlinkage void process_monitor_reg_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    
    enum proc_monitor_type type = (enum proc_monitor_type)arg->msg->params[0].value;
    unsigned long func_num = arg->msg->params[1].value;
    unsigned long opcode = arg->msg->params[2].value;
    
    register_process_monitor(type, t->proc_id, func_num, opcode);
    
    // Resume sender thread
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}
