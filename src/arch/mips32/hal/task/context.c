#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "common/include/proc.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
//#include "hal/include/task.h"


dec_per_cpu(ulong, cur_running_sched_id);

dec_per_cpu(int, cur_in_user_mode);
dec_per_cpu(struct context, cur_context);


void init_thread_context(struct context *context, ulong entry, ulong stack_top, int user_mode)
{

}

u32 asmlinkage save_context(struct context *context)
{

}

static void no_opt switch_page_dir(ulong page_dir_pfn)
{

}

static void no_opt switch_to_user(struct context *context)
{

}

static void no_opt switch_to_kernel(struct context *context)
{

}

void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid,
                                      struct thread_control_block *tcb)
{

}

void init_context_mp()
{
    ulong *sched_id = get_per_cpu(ulong, cur_running_sched_id);
    *sched_id = 0;
    
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    *user_mode = 0;
}

void init_context()
{
    ulong *sched_id = get_per_cpu(ulong, cur_running_sched_id);
    *sched_id = 0;
    
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    *user_mode = 0;
}
