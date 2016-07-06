#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"


/*
 * Thread creation and termination
 */
unsigned long kapi_thread_create(kapi_thread_start_routine start_routine, void *arg)
{
    msg_t *s = kapi_msg(KAPI_THREAD_CREATE);
    msg_t *r;
    unsigned long result = 0;
    
    msg_param_value(s, (unsigned long)start_routine);
    msg_param_value(s, (unsigned long)arg);
    r = syscall_request();
    result = (int)kapi_return_value(r);
    
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
    msg_t *s = kapi_msg(KAPI_THREAD_ID);
    msg_t *r;
    unsigned long result = 0;
    
    r = syscall_request();
    result = kapi_return_value(r);
    
    return result;
}
