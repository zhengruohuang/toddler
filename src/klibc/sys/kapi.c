#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"


/*
 * Helper functions
 */
msg_t *kapi_msg(int kapi_num)
{
    msg_t *msg = syscall_msg();
    
    msg->mailbox_id = IPC_MAILBOX_KERNEL;
    msg->opcode = IPC_OPCODE_KAPI;
    msg->func_num = kapi_num;
    
    return msg;
}

unsigned long kapi_return_value(msg_t *m)
{
    return m->params[0].value;
}

void msg_param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = msg_param_type_value;
    m->params[index].value = value;
    
    m->param_count++;
}

void msg_param_buffer(msg_t *m, void *buf, size_t size)
{
    int index = m->param_count;
    unsigned char *src = NULL;
    unsigned char *dest = NULL;
    unsigned char temp = 0;
    int i;
    
    // Align the size
    int extra_msg_size = (int)size;
    if (extra_msg_size % (int)sizeof(unsigned long)) {
        extra_msg_size /= (int)sizeof(unsigned long);
        extra_msg_size++;
        extra_msg_size *= (int)sizeof(unsigned long);
    }
    
    // Set param
    m->params[index].type = msg_param_type_buffer;
    m->params[index].offset = m->msg_size;
    m->params[index].size = (int)size;
    
    // Copy the buffer content
    src = (unsigned char *)buf;
    dest = ((unsigned char *)m) + m->msg_size;
    
    for (i = 0; i < size; i++) {
        *dest++ = *src++;
    }
    
    // Set size and count
    m->msg_size += extra_msg_size;
    m->param_count++;
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
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_THREAD_EXIT);
    msg_param_value(s, (unsigned long)retval);
    
    // Issue the KAPI
    syscall_request();
    
    // Should never reach here
    do {
        // Nothing
    } while (1);
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
    msg_t *r = NULL;
    int sys_ret = 0;
    int result = 0;
    
    // Setup the params
    msg_param_value(s, (unsigned long)fd);
    msg_param_buffer(s, buf, count);
    msg_param_value(s, (unsigned long)count);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    
    // Setup the result
    result = (int)kapi_return_value(r);
    
    return result;
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

