#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"


static unsigned long kthread_tls_offset = -1;


/*
 * Thread creation
 */
static asmlinkage void kthread_start_wrapper(msg_t *msg)
{
    // Get params
    start_routine_t start_routine = (start_routine_t)msg->params[0].value;
    unsigned long start_arg = (unsigned long)msg->params[1].value;
    kthread_t *thread = (kthread_t *)msg->params[2].value;
    unsigned long ret = 0;
    
    // Save the addr of kthread obj
    *((kthread_t **)ktls_access(kthread_tls_offset)) = thread;
    
    // Set state
    thread->started = 1;
    
    // Execute the routine
    ret = start_routine(start_arg);
    
    // Set state
    if (thread) {
        thread->return_value = ret;
        thread->terminated = 1;
    }
    
    // Done
    kapi_thread_exit(NULL);
}

int kthread_create(kthread_t *thread, start_routine_t start, unsigned long arg)
{
    thread->return_value = 0;
    thread->started = 0;
    thread->terminated = 0;
    
    return 0;
}


/*
 * Thread termination
 */
static asmlinkage void kthread_kill_wrapper(unsigned long retval)
{
    kthread_t *thread = *((kthread_t **)ktls_access(kthread_tls_offset));
    thread->return_value = retval;
    thread->terminated = 1;
    
    kapi_thread_exit(NULL);
}

void kthread_exit(unsigned long retval)
{
    kthread_kill_wrapper(retval);
}

void kthread_kill(kthread_t *thread, unsigned long retval)
{
}


/*
 * Initialization
 */
void init_kthread()
{
    kthread_tls_offset = ktls_alloc(sizeof(unsigned long *));
    //assert(thread_return_value_tls_id > 0);
}
