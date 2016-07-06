#include "common/include/data.h"
#include "common/include/syscall.h"
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
unsigned long kapi_process_id()
{
    msg_t *s = kapi_msg(KAPI_PROCESS_ID);
    msg_t *r;
    unsigned long result = 0;
    
    r = syscall_request();
    result = kapi_return_value(r);
    
    return result;
}


//  char **environ;


//  int fork();
//  int execve(char *name, char **argv, char **env);
//  int kill(int pid, int sig);
//  int wait(int *status);
//  caddr_t sbrk(int incr);
