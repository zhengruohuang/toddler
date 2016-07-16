/*
 * KAPI Handling - Thread
 */
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/kapi.h"


asmlinkage void thread_create_handler(struct kernel_msg_handler_arg *arg)
{
    struct process *p = arg->sender_thread->proc;
    struct thread *t;
    msg_t *m;
    
    ulong wrapper = arg->msg->params[0].value;
    ulong stack_size = arg->msg->params[1].value;
    ulong tls_size = arg->msg->params[2].value;
    
    ulong start_routine = arg->msg->params[3].value;
    ulong start_arg = arg->msg->params[4].value;
    ulong kthread = arg->msg->params[5].value;
    
    //kprintf("To create user thread, process: %s, wrapper: %p, stack: %p, tls: %p\n", p->name, wrapper, stack_size, tls_size);
    
    // Create the thread
    t = create_thread(p, wrapper, 0, -1, stack_size, tls_size);
    assert(t);
    
    // Set the msg for the newly created thread
    m = create_response_msg(t);
    set_msg_param_value(m, start_routine);
    set_msg_param_value(m, start_arg);
    set_msg_param_value(m, kthread);
    
    set_thread_arg(t, t->memory.block_base + t->memory.msg_recv_offset);
    
    // Set the msg for the sender thread
    m = create_response_msg(arg->sender_thread);
    set_msg_param_value(m, t->thread_id);
    
    // Run both threads
    run_thread(t);
    run_thread(arg->sender_thread);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);

    // Wait for this thread to be terminated
    kernel_unreachable();
}

asmlinkage void thread_exit_handler(struct kernel_msg_handler_arg *arg)
{
    //kprintf("To terminate user thread: %p, process: %s\n", arg->sender_thread, arg->sender_thread->proc->name);
    
    terminate_thread(arg->sender_thread);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

asmlinkage void thread_kill_handler(struct kernel_msg_handler_arg *arg)
{
    kprintf("To terminate 3rd user thread: %p, process: %s\n", arg->sender_thread, arg->sender_thread->proc->name);
    
    ulong thread_id = arg->msg->params[0].value;
    
    struct thread *t = gen_thread_by_thread_id(thread_id);
    assert(t);
    
    terminate_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}
