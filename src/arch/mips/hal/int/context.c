#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "common/include/context.h"
#include "common/include/proc.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/percpu.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"


dec_per_cpu(ulong, cur_running_sched_id);

dec_per_cpu(int, cur_in_user_mode);
dec_per_cpu(struct saved_context, cur_context);

dec_per_cpu(ulong, cur_tcb_vaddr);


/*
 * Initialize context for a newly created thread
 */
void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode)
{
    // Set GPRs
    memzero(context, sizeof(struct context));
    
    // Set param
    context->a0 = param;
    
    // Set other states
    context->sp = stack_top;
    context->pc = entry;
    context->delay_slot = 0;
    
    if (!user_mode) {
        context->sp |= SEG_CACHED;
    }
}

void set_thread_context_param(struct context *context, ulong param)
{
    context->a0 = param;
}


/*
 * Save context for interrupt handling
 */
u32 asmlinkage save_context(struct context *context)
{
    // Set local interrupt state to disabled
    disable_local_int();
    
    // Save PC and branch delay slot state
    read_cp0_epc(context->pc);
    
    // Save branch slot state
    struct cp0_cause cause;
    read_cp0_cause(cause.value);
    context->delay_slot = cause.bd;
}


/*
 * Context switch
 */
static void no_opt switch_page_dir(ulong page_dir_pfn)
{
    struct page_frame **page = get_per_cpu(struct page_frame *, cur_page_dir);
    *page = (struct page_frame *)PFN_TO_ADDR(page_dir_pfn);
}

static void no_opt switch_to(struct context *context, int user_mode, ulong asid)
{
    // Set EPC to PC
    write_cp0_epc(context->pc);
    
    // Set CAUSE based on delay slot state
    struct cp0_cause cause;
    read_cp0_cause(cause.value);
    cause.bd = context->delay_slot ? 1 : 0;
    write_cp0_cause(cause.value);
    
    // Set SR based on user/kernel mode, also set EXL bit - switching is enabled
    struct cp0_status status;
    read_cp0_status(status.value);
    status.ksu = user_mode ? 0x2 : 0;
    status.exl = 1;
    status.erl = 0;
    write_cp0_status(status.value);
    
    // Set ASID in TLB EntryHi
    struct cp0_entry_hi hi;
    read_cp0_entry_hi(hi.value);
    hi.asid = asid;
    write_cp0_entry_hi(hi.value);
    
    // Restore GPRs
    restore_context_gpr();
}

void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb)
{
    kprintf("To switch context, PC: %lx, SP: %lx, ASID: %lx, user: %d\n",
           context->pc, context->sp, asid, user_mode);
    
    // Disable local interrupts
    disable_local_int();
    
    // Mark local interrupt state as enabled
    set_local_int_state(1);
    
    // Copy the context to local, thus prevent TLB miss
    struct saved_context *per_cpu_context = get_per_cpu(struct saved_context, cur_context);
    memcpy(&per_cpu_context->context, context, sizeof(struct context));
    
    // Load context addr to k0
    __asm__ __volatile__ (
        "move   $26, %[ctx];"
        :
        : [ctx] "r" ((ulong)per_cpu_context)
    );
    
    // Set sched id
    *get_per_cpu(ulong, cur_running_sched_id) = sched_id;
    *get_per_cpu(int, cur_in_user_mode) = user_mode;
    
    // Make k1 ponit to TCB
    __asm__ __volatile__ (
        "move   $27, %0;"
        :
        : "r" (tcb)
    );
    
    *(ulong *)get_per_cpu(ulong, cur_tcb_vaddr) = tcb;
    
    // Switch page dir
    switch_page_dir(page_dir_pfn);
    
    // Do the actual switch
    switch_to(context, user_mode, asid);
}

void init_context_mp()
{
    ulong *sched_id = get_per_cpu(ulong, cur_running_sched_id);
    *sched_id = 0;
    
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    *user_mode = 0;
    
    ulong *cur_tcb = get_per_cpu(ulong, cur_tcb_vaddr);
    *cur_tcb = 0;
}

void init_context()
{
    ulong *sched_id = get_per_cpu(ulong, cur_running_sched_id);
    *sched_id = 0;
    
    int *user_mode = get_per_cpu(int, cur_in_user_mode);
    *user_mode = 0;
    
    ulong *cur_tcb = get_per_cpu(ulong, cur_tcb_vaddr);
    *cur_tcb = 0;
//     
//     kprintf("Here, cur_tcb: %p\n", cur_tcb);
//     while (1);
}
