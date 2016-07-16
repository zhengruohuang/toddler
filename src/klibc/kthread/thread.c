#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"


static unsigned long kthread_tls_offset = 0;


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
    atomic_membar();
    
    thread->thread_id = kapi_thread_create(
        kthread_start_wrapper, 0, 0,
        3, (unsigned long)start, (unsigned long)arg, (unsigned long)thread
    );
    atomic_membar();
    
    return thread->thread_id ? 1 : 0;
}


/*
 * Thread info
 */
kthread_t *kthread_self()
{
    kthread_t *thread = *((kthread_t **)ktls_access(kthread_tls_offset));
    return thread;
}


/*
 * Thread termination
 */
void kthread_exit(unsigned long retval)
{
    kthread_t *thread = *((kthread_t **)ktls_access(kthread_tls_offset));
    
    thread->return_value = retval;
    thread->terminated = 1;
    atomic_membar();
    
    kapi_thread_exit(NULL);
}

void kthread_kill(kthread_t *thread, unsigned long retval)
{
    if (!thread || thread->terminated) {
        return;
    }
    
    kpai_thread_kill(thread->thread_id);
    
    thread->return_value = retval;
    thread->terminated = 1;
    atomic_membar();
}


/*
 * Initialization
 */
void init_kthread()
{
    kthread_tls_offset = ktls_alloc(sizeof(unsigned long *));
    //assert(kthread_tls_offset);
}
