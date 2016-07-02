#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"


//  int open(const char *name, int flags, ...);
//  int close(int fd); 
//  int kapi_read(int fd, char *buf, size_t count);

asmlinkage void kapi_write_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    
    int fd = (int)msg->params[0].value;
    size_t count = (size_t)msg->params[2].value;
    
    //     vsnprintf(buf, 128, "I got a msg, mailbox_id: %p\n", reply_mbox_id);
    //     syscall_kputs(buf);
    //     
    //     vsnprintf(buf, 128, "fd: %d, count: %p, offset: %d, size: %d\n",
    //               fd, count, msg->params[1].offset, msg->params[1].size
    //     );
    //     syscall_kputs(buf);
    //     
    //     vsnprintf(buf, 128, "param0 type: %d, param1: %d, param2: %d\n",
    //               msg->params[0].type, msg->params[1].type, msg->params[2].type
    //     );
    //     syscall_kputs(buf);
    //     
    //     int i;
    //     char *src_buf = (char *)((unsigned long)msg + msg->params[1].offset);
    //     for (i = 0; i < msg->params[1].size; i++) {
    //         buf[i] = src_buf[i];
    //     }
    //     buf[128] = '\0';
    //     
    //     syscall_kputs(buf);
    
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
