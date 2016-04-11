#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"


extern struct thread_control_block *get_tcb();
extern int do_syscall(unsigned long num, unsigned long param, unsigned long *out1, unsigned long *out2);

extern int syscall_ping(unsigned long ping, unsigned long *pong);
extern int syscall_kputs(char *s);

extern msg_t *syscall_msg();

extern int syscall_send(msg_t *msg);
extern msg_t *syscall_recv();

extern msg_t *syscall_request(msg_t *msg);
extern int syscall_reply(msg_t *in_msg, msg_t *out_msg);
