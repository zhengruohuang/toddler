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
extern int syscall_yield();

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

/*
 * Process
 */
extern void kapi_process_exit(int err_code);
extern int kpai_process_kill(unsigned long process_id);
extern unsigned long kapi_process_id();

/*
 * Thread
 */
extern unsigned long kapi_thread_create(msg_handler_t wrapper, unsigned long stack_size, unsigned long tls_size, int arg_count, ...);
extern void kapi_thread_exit(void *retval);
extern int kpai_thread_kill(unsigned long thread_id);
extern unsigned long kapi_thread_id();

/*
 * File
 */
extern unsigned long kapi_file_open(char *name, int mode);
extern int kapi_file_close(unsigned long fd);
extern int kapi_file_read(unsigned long fd, char *buf, size_t count);
extern int kapi_file_write(unsigned long fd, void *buf, size_t count);

/*
 * Interrupt
 */
extern int kapi_interrupt_reg(unsigned long irq, void *handler_entry);
extern int kapi_interrupt_unreg(unsigned long irq);

/*
 * Heap
 */
extern unsigned long kapi_get_heap_end();
extern unsigned long kapi_set_heap_end(unsigned long heap_end);
extern unsigned long kapi_grow_heap(unsigned long amount);
extern unsigned long kapi_shrink_heap(unsigned long amount);

extern int kapi_brk(unsigned long heap_end);
extern unsigned long kapi_sbrk(long amount);


#endif
