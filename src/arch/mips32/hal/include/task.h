#ifndef __ARCH_MIPS32_HAL_INCLUDE_TASK__
#define __ARCH_MIPS32_HAL_INCLUDE_TASK__


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/proc.h"
#include "hal/include/cpu.h"


/*
 * Context
 */
struct saved_context {
    struct context context;
    u32 tlb_refill_sp;
    u32 kernel_sp;
} packedstruct;

ext_per_cpu(ulong, cur_running_sched_id);

ext_per_cpu(int, cur_in_user_mode);
ext_per_cpu(struct saved_context, cur_context);

extern void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode);
extern void set_thread_context_param(struct context *context, ulong param);
extern u32 asmlinkage save_context(struct context *context);
extern void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb);
extern void init_context_mp();
extern void init_context();


/*
 * Restore
 */
extern void restore_context_gpr();


#endif
