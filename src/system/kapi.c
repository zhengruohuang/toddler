#include "common/include/data.h"
#include "common/include/syscall.h"
#include "system/include/syscall.h"


/*
 * Helper functions
 */
static msg_t *kapi_msg(int kapi_num)
{
    msg_t *msg = syscall_msg();
    
    msg->dest_mailbox_id = IPC_DEST_KERNEL;
    msg->msg_num = kapi_num;
    msg->need_response = 1;
    msg->param_count = 0;
    
    return msg;
}

static void kapi_param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = msg_param_value;
    m->params[index].value = value;
    
    m->param_count++;
}

static void kapi_param_ptr(msg_t *m, void *p, unsigned long size, int writable)
{
    int index = m->param_count;
    
    m->params[index].type = writable ? msg_param_addr_rw : msg_param_addr_ro;
    m->params[index].vaddr = p;
    m->params[index].size = size;
    
    m->param_count++;
}

static unsigned long kapi_return_value(msg_t *m)
{
    return m->params[0].value;
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
 * Thread
 */
void thread_exit(void *retval)
{
}

/*
 * File
 */
//  int open(const char *name, int flags, ...);
//  int close(int fd); 

int kapi_read(int fd, char *buf, size_t count)
{
    return 0;
}

int kapi_write(int fd, void *buf, size_t count)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_WRITE);
    
    // Setup the params
    kapi_param_value(s, (unsigned long)fd);
    kapi_param_ptr(s, (void *)buf, count, 0);
    kapi_param_value(s, (unsigned long)count);
    
    // Issue the KAPI and obtain the result
    msg_t *r = syscall_request(s);
    
    // Setup the result
    int result = (int)kapi_return_value(r);
    
    return result;
}

asmlinkage void kapi_write_handler(msg_t *msg)
{
    thread_exit(NULL);
}

//  int lseek(int file, int ptr, int dir);

//  int fstat(int file, struct stat *st);
//  int stat(const char *file, struct stat *st);

//  int link(char *old, char *new);
//  int unlink(char *name);

//  int isatty(int file);


/*
 * Time
 */
//  clock_t times(struct tms *buf);
//  int gettimeofday(struct timeval *p, struct timezone *z);


/*
 * Initialization
 */
int kapi_init()
{
    int reg = 0;
    
    reg += syscall_reg_msg_handler(KAPI_WRITE, kapi_write_handler);
    
    return reg;
}
