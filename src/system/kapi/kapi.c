#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "system/include/kapi.h"


void init_kapi()
{
//     kapi_reg(KAPI_FILE_WRITE, kapi_write_handler);
    
    // URS
    kapi_reg(KAPI_URS_REG_SUPER, urs_reg_super_handler);
    
    kapi_reg(KAPI_URS_OPEN, urs_open_handler);
    kapi_reg(KAPI_URS_CLOSE, urs_close_handler);
    kapi_reg(KAPI_URS_READ, urs_read_handler);
    kapi_reg(KAPI_URS_WRITE, urs_write_handler);
    kapi_reg(KAPI_URS_LIST, urs_list_handler);
    
    kapi_reg(KAPI_URS_CREATE, urs_create_handler);
    kapi_reg(KAPI_URS_REMOVE, urs_remove_handler);
    kapi_reg(KAPI_URS_RENAME, urs_rename_handler);
    
    kapi_reg(KAPI_URS_STAT, urs_stat_handler);
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

