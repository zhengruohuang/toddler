#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "klibc/include/sys.h"


/*
 * Process creation and termination
 */
void kapi_process_exit(int err_code)
{
    msg_t *s = kapi_msg(KAPI_PROCESS_EXIT);
    
    msg_param_value(s, (unsigned long)err_code);
    syscall_request();
    
    sys_unreahable();
}

int kpai_process_kill(unsigned long process_id)
{
    msg_t *s = kapi_msg(KAPI_PROCESS_KILL);
    msg_t *r;
    int result = -1;
    
    msg_param_value(s, process_id);
    r = syscall_request();
    result = (int)kapi_return_value(r);
    
    return result;
}


/*
 * Process info
 */
void kapi_process_started(unsigned long code)
{
    msg_t *s = kapi_msg(KAPI_PROCESS_STARTED);
    msg_param_value(s, code);
    syscall_request();
}

unsigned long kapi_process_id()
{
    struct thread_control_block *tcb = get_tcb();
    return tcb->proc_id;
}


/*
 * Process monitor
 */
int kapi_process_monitor(enum proc_monitor_type type, unsigned long func_num, unsigned long opcode)
{
    msg_t *s = kapi_msg(KAPI_PROCESS_MONITOR);
    msg_t *r;
    int result = -1;
    
    msg_param_value(s, (unsigned long)type);
    msg_param_value(s, func_num);
    msg_param_value(s, opcode);
    r = syscall_request();
    result = (int)kapi_return_value(r);
    
    return result;
}


//  char **environ;


//  int fork();
//  int execve(char *name, char **argv, char **env);
//  int kill(int pid, int sig);
//  int wait(int *status);
//  caddr_t sbrk(int incr);
