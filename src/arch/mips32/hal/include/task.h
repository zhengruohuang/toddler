#ifndef __ARCH_MIPS32_HAL_INCLUDE_TASK__
#define __ARCH_MIPS32_HAL_INCLUDE_TASK__


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/proc.h"
#include "hal/include/cpu.h"


/*
 * Context
 */
ext_per_cpu(ulong, cur_running_sched_id);

ext_per_cpu(int, cur_in_user_mode);
ext_per_cpu(struct context, cur_context);

extern void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid,
                                      struct thread_control_block *tcb);
extern void init_context_mp();
extern void init_context();


/*
 * Restore
 */
extern void restore_context_gpr();


#endif
