/*
 * KAPI Handling - URS
 */

#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"
#include "kernel/include/urs.h"
#include "kernel/include/kapi.h"


asmlinkage void urs_open_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    char *name = (char *)((ulong)s + s->params[0].offset);
    int mode = (int)s->params[1].value;
    
    int result = (int)urs_open_node(name, mode, t->proc_id);
    set_msg_param_value(r, (ulong)result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

asmlinkage void urs_close_handler(struct kernel_msg_handler_arg *arg)
{
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong open_id = s->params[0].value;
    int result = (int)urs_close_node(open_id, t->proc_id);
    set_msg_param_value(r, (ulong)result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

asmlinkage void urs_read_handler(struct kernel_msg_handler_arg *arg)
{
    u8 read_buf[128];
    
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong open_id = s->params[0].value;
    int buf_size = (int)s->params[1].value;
    if (buf_size > sizeof(read_buf)) {
        buf_size = sizeof(read_buf);
    }
    
    ulong len = 0;
    int result = (int)urs_read_node(open_id, read_buf, buf_size, &len);
    set_msg_param_buf(r, read_buf, len);
    set_msg_param_value(r, len);
    set_msg_param_value(r, (ulong)result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}

asmlinkage void urs_list_handler(struct kernel_msg_handler_arg *arg)
{
    u8 name_buf[128];
    
    struct thread *t = arg->sender_thread;
    msg_t *s = arg->msg;
    msg_t *r = create_response_msg(t);
    
    ulong open_id = s->params[0].value;
    int buf_size = (int)s->params[1].value;
    if (buf_size > sizeof(name_buf)) {
        buf_size = sizeof(name_buf);
    }
    
    ulong len = 0;
    int result = (int)urs_list_node(open_id, name_buf, buf_size, &len);
    set_msg_param_buf(r, name_buf, len);
    set_msg_param_value(r, len);
    set_msg_param_value(r, (ulong)result);
    
    run_thread(t);
    
    // Clean up
    terminate_thread_self(arg->handler_thread);
    sfree(arg);
    
    // Wait for this thread to be terminated
    kernel_unreachable();
}
