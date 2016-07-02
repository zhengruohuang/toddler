#ifndef __KLIBC_INCLUDE_SYS__
#define __KLIBC_INCLUDE_SYS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"


/*
 * System
 */
extern void sys_unreahable();


/*
 * System call
 */
extern struct thread_control_block *get_tcb();
extern int do_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2);

extern int syscall_ping(unsigned long ping, unsigned long *pong);
extern int syscall_kputs(char *s);

extern msg_t *syscall_msg();

extern int syscall_send();
extern int syscall_reply();
extern msg_t *syscall_recv();

extern msg_t *syscall_request();
extern int syscall_respond();

extern int syscall_reg_msg_handler(unsigned long msg_num, msg_handler_t msg_handler);
extern int syscall_unreg_msg_handler(unsigned long msg_num);

extern int syscall_reg_kapi_server(unsigned long kapi_num);
extern int syscall_unreg_kapi_server(unsigned long kapi_num);


/*
 * KAPI
 */
extern msg_t *kapi_msg(int kapi_num);
extern unsigned long kapi_return_value(msg_t *m);
extern void msg_param_value(msg_t *m, unsigned long value);
extern void msg_param_buffer(msg_t *m, void *buf, size_t size);

extern void thread_exit(void *retval);

extern int kapi_read(int fd, char *buf, size_t count);
extern int kapi_write(int fd, void *buf, size_t count);


#endif
