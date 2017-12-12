#ifndef __ARCH_SPARC_HAL_INCLUDE_INT__
#define __ARCH_SPARC_HAL_INCLUDE_INT__


#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "common/include/proc.h"
#include "hal/include/percpu.h"


/*
 * Handling type
 */
#define INT_HANDLE_TYPE_TAKEOVER    0
#define INT_HANDLE_TYPE_HAL         1
#define INT_HANDLE_TYPE_KERNEL      2


/*
 * Generic interrupt
 */
extern void init_int();

extern int get_local_int_state();
extern void set_local_int_state(int enabled);

extern int disable_local_int();
extern void enable_local_int();
extern void restore_local_int(int enabled);

// extern void init_int_state_mp();
// extern void init_int_state();


/*
 * Start working
 */
extern volatile int work_started;
extern void start_working();


/*
 * Handler template
 */
extern int __int_entry_wrapper_begin;
extern int __int_entry_wrapper_end;


/*
 * Syscall
 */
extern void init_syscall();
extern void set_syscall_return(struct context *context, int succeed, ulong return0, ulong return1);


/*
 * Page fault
 */
extern void init_pagefault();


/*
 * Context
 */
ext_per_cpu(ulong, cur_running_sched_id);

ext_per_cpu(int, cur_in_user_mode);
ext_per_cpu(struct saved_context, cur_context);
ext_per_cpu(ulong, cur_asid);

ext_per_cpu(ulong, cur_tcb_vaddr);

extern void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode);
extern void set_thread_context_param(struct context *context, ulong param);

extern void asmlinkage save_context(struct context *context);
extern void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb);

extern void init_context_mp();
extern void init_context();


/*
 * Restore
 */
extern void restore_context_gpr(struct context *context, ulong vsid_base);


#endif
