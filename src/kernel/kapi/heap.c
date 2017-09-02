/*
 * KAPI Handling - Heap
 */
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


asmlinkage void set_heap_end_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong heap_end = s->params[0].value;
    ulong result = set_heap_end(t->proc, heap_end);
    set_msg_param_value(r, result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    ksys_unreachable();
}

asmlinkage void get_heap_end_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *r = create_response_msg(t);
    
    ulong result = get_heap_end(t->proc);
    set_msg_param_value(r, result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    ksys_unreachable();
}

asmlinkage void grow_heap_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong amount = s->params[0].value;
    ulong result = grow_heap(t->proc, amount);
    set_msg_param_value(r, result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    ksys_unreachable();
}

asmlinkage void shrink_heap_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong amount = s->params[0].value;
    ulong result = shrink_heap(t->proc, amount);
    set_msg_param_value(r, result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    ksys_unreachable();
}
