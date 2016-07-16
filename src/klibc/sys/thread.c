#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/stdarg.h"
#include "klibc/include/sys.h"


/*
 * Thread creation and termination
 */
unsigned long kapi_thread_create(msg_handler_t wrapper, unsigned long stack_size, unsigned long tls_size, int arg_count, ...)
{
    msg_t *s = kapi_msg(KAPI_THREAD_CREATE);
    msg_t *r;
    va_list ap;
    unsigned long arg;
    unsigned long result = 0;
    
    // Set up wrapper routine
    msg_param_value(s, (unsigned long)wrapper);
    msg_param_value(s, stack_size);
    msg_param_value(s, tls_size);
    
    // Set up args
    if (arg_count) {
        va_start(ap, arg_count);
        do {
            arg = va_arg(ap, unsigned long);
            msg_param_value(s, arg);
        } while (--arg_count);
        va_end(ap);
    }
    
    // Issue the KAPI
    r = syscall_request();
    result = kapi_return_value(r);
    
    return result;
}

void kapi_thread_exit(void *retval)
{
    msg_t *s = kapi_msg(KAPI_THREAD_EXIT);
    
    msg_param_value(s, (unsigned long)retval);
    syscall_request();
    
    sys_unreahable();
}

int kpai_thread_kill(unsigned long thread_id)
{
    msg_t *s = kapi_msg(KAPI_THREAD_KILL);
    msg_t *r;
    int result = -1;
    
    msg_param_value(s, thread_id);
    r = syscall_request();
    result = (int)kapi_return_value(r);
    
    return result;
}


/*
 * Thread info
 */
unsigned long kapi_thread_id()
{
    struct thread_control_block *tcb = get_tcb();
    return tcb->thread_id;
}
