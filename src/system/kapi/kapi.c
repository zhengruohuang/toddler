#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "system/include/kapi.h"


static void reg_kapi_handler(unsigned long kapi_num, msg_handler_t handler)
{
    syscall_reg_kapi_server(kapi_num);
    syscall_reg_msg_handler(kapi_num, handler);
}

void init_kapi()
{
    reg_kapi_handler(KAPI_FILE_WRITE, kapi_write_handler);
}


//  char **environ;

/*
 * Process
 */
//  int fork();
//  int execve(char *name, char **argv, char **env);
//  int kill(int pid, int sig);
//  int wait(int *status);
//  caddr_t sbrk(int incr);
//  void _exit();
//  int getpid();


/*
 * Time
 */
//  clock_t times(struct tms *buf);
//  int gettimeofday(struct timeval *p, struct timezone *z);

