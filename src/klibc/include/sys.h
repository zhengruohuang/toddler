#ifndef __KLIBC_INCLUDE_SYS__
#define __KLIBC_INCLUDE_SYS__


#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "common/include/urs.h"


/*
 * Klib
 */
extern void klib_init_thread();


/*
 * System
 */
extern void sys_unreahable();
extern void sys_yield();


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
 * User message
 */
extern unsigned long alloc_msg_num();


/*
 * KAPI
 */
extern msg_t *kapi_msg(int kapi_num);
extern unsigned long kapi_return_value(msg_t *m);
extern unsigned long msg_return_value(msg_t *m);
extern void *msg_return_buffer(msg_t *m, size_t *size);
extern void msg_param_value(msg_t *m, unsigned long value);
extern void msg_param_buffer(msg_t *m, void *buf, size_t size);
extern int kapi_reg(unsigned long kapi_num, msg_handler_t handler);

/*
 * Process
 */
extern void kapi_process_exit(int err_code);
extern int kpai_process_kill(unsigned long process_id);
extern void kapi_process_started(unsigned long code);
extern unsigned long kapi_process_id();

/*
 * Thread
 */
extern unsigned long kapi_thread_create(msg_handler_t wrapper, unsigned long stack_size, unsigned long tls_size, int arg_count, ...);
extern void kapi_thread_exit(void *retval);
extern int kpai_thread_kill(unsigned long thread_id);
extern unsigned long kapi_thread_id();

/*
 * URS
 */
extern unsigned long kapi_urs_reg_super(char *path, char *name, unsigned int flags, struct urs_reg_ops *ops);
extern int kapi_urs_reg_op(unsigned long super_id, enum urs_op_type op, unsigned long msg_opcode, unsigned long msg_func_num);

extern unsigned long kapi_urs_open(char *name, unsigned int flags);
extern int kapi_urs_close(unsigned long fd);
extern size_t kapi_urs_read(unsigned long fd, void *buf, size_t count);
extern size_t kapi_urs_write(unsigned long fd, void *buf, size_t count);
extern size_t kapi_urs_list(unsigned long fd, void *buf, size_t count);

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

/*
 * Stdio
 */
extern unsigned long kapi_stdin_read(unsigned long console_id, void *buf, unsigned long count);
extern unsigned long kapi_stdout_write(unsigned long console_id, void *buf, unsigned long count);
extern unsigned long kapi_stderr_write(unsigned long console_id, void *buf, unsigned long count);


#endif
