/*
 * KAPI Handling - KMap
 */
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


/*
 * KMap
 */
asmlinkage void kmap_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    struct process *p = t->proc;
    
    // Do KMap
    enum kmap_region region = (enum kmap_region)arg->msg->params[0].value;
    unsigned long vaddr = kmap(p, region);
    
    // Reply
    msg_t *m = create_response_msg(t);
    set_msg_param_value(m, vaddr);
    
    // Resume caller thread
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}
