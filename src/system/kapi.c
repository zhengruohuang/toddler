#include "common/include/data.h"
#include "common/include/syscall.h"
#include "system/include/syscall.h"


/*
 * Helper functions
 */
static msg_t *kapi_msg(int kapi_num)
{
    msg_t *msg = syscall_msg();
    
    msg->mailbox_id = IPC_MAILBOX_KERNEL;
    msg->opcode = IPC_OPCODE_KAPI;
    msg->func_num = kapi_num;
    
    return msg;
}

static void param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = msg_param_value;
    m->params[index].value = value;
    
    m->param_count++;
}

static void param_buffer(msg_t *m, void *buf, size_t size)
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
    m->params[index].type = msg_param_buffer;
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

static unsigned long return_value(msg_t *m)
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
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_THREAD_EXIT);
    
    //syscall_kputs("To set up the msg!\n");
    
    // Setup the params
    param_value(s, (unsigned long)retval);
    
    syscall_kputs("To call thread exit syscall!\n");
    
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    // Issue the KAPI
    syscall_request();
    
    // Should never reach here
    do {
        // Nothing
    } while (1);
}

extern int asmlinkage vsnprintf(char *buf, size_t size, char *fmt, ...);

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
    
    //syscall_kputs("To set up the msg!\n");
    
//     vsnprintf((char *)buf, 128, "fd: %d, count: %p, buf: %p\n",
//               fd, count, buf
//     );
//     syscall_kputs((char *)buf);
    
    // Setup the params
    param_value(s, (unsigned long)fd);
    param_buffer(s, buf, count);
    param_value(s, (unsigned long)count);
    
//     vsnprintf((char *)buf, 128, "fd: %d, count: %p, buf: %p\n",
//               fd, count, buf
//     );
//     syscall_kputs((char *)buf);
    
    syscall_kputs("To call syscall request!\n");
    
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    
    // Setup the result
    //result = (int)return_value(r);
    
    return result;
}

static char buf[128];

asmlinkage void kapi_write_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    
    int fd = (int)msg->params[0].value;
    size_t count = (size_t)msg->params[2].value;
    
    vsnprintf(buf, 128, "I got a msg, mailbox_id: %p\n", reply_mbox_id);
    syscall_kputs(buf);
    
    vsnprintf(buf, 128, "fd: %d, count: %p, offset: %d, size: %d\n",
              fd, count, msg->params[1].offset, msg->params[1].size
    );
    syscall_kputs(buf);
    
    vsnprintf(buf, 128, "param0 type: %d, param1: %d, param2: %d\n",
              msg->params[0].type, msg->params[1].type, msg->params[2].type
    );
    syscall_kputs(buf);
    
    int i;
    char *src_buf = (char *)((unsigned long)msg + msg->params[1].offset);
    for (i = 0; i < msg->params[1].size; i++) {
        buf[i] = src_buf[i];
    }
    buf[128] = '\0';
    
    syscall_kputs(buf);
    
    // Setup the msg
    msg_t *s = syscall_msg();
    s->mailbox_id = reply_mbox_id;
    
    syscall_kputs("To call syscall respond!\n");
    
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    // Issue the KAPI and obtain the result
    syscall_respond();
    
    // Should never reach here
    do {
        // Nothing
    } while (1);
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
    
    syscall_reg_kapi_server(KAPI_WRITE);
    reg += syscall_reg_msg_handler(KAPI_WRITE, kapi_write_handler);
    
    return reg;
}
