#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "common/include/context.h"
#include "common/include/proc.h"
#include "common/include/reg.h"
#include "common/include/atomic.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/percpu.h"
#include "hal/include/string.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"


dec_per_cpu(ulong, cur_running_sched_id);

dec_per_cpu(int, cur_in_user_mode);
dec_per_cpu(struct context, cur_context);

dec_per_cpu(ulong, cur_tcb_vaddr);

dec_per_cpu(struct l1table *, cur_page_dir);


static void stop(ulong val)
{
    if (!val) {
        val = 0xbeef;
    }
    
    while (1) {
        __asm__ __volatile__
        (
            "mov r0, %[reg];"
            :
            : [reg] "r" (val)
        );
    }
}



/*
 * Initialize context for a newly created thread
 */
void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode)
{
    // Set GPRs
    memzero(context, sizeof(struct context));
    
    // Set param
    context->r0 = param;
    
    // Set PC and SP
    context->sp = stack_top;
    context->pc = entry;
    
    // Set CPSR
    struct proc_status_reg psr;
    psr.value = 0;
    psr.mode = user_mode ? 0x10 : 0x1f;
    context->cpsr = psr.value;
}

void set_thread_context_param(struct context *context, ulong param)
{
    context->r0 = param;
}


/*
 * Context switch
 */
static void no_opt switch_page_dir(ulong page_dir_pfn)
{
    struct l1table **pl1tab = get_per_cpu(struct l1table *, cur_page_dir);
    struct l1table *page_dir = (struct l1table *)PFN_TO_ADDR(page_dir_pfn);
    *pl1tab = page_dir;
    
//     kprintf("To switch page dir PFN @ %lx\n", page_dir_pfn);
    
    write_trans_tab_base0(page_dir);
    inv_tlb_all();
    atomic_membar();
}

static void no_opt switch_to(struct context *context, int user_mode, ulong asid)
{
//     // Set EPC to PC
//     write_cp0_epc(context->pc);
//     
//     // Set CAUSE based on delay slot state
//     struct cp0_cause cause;
//     read_cp0_cause(cause.value);
//     cause.bd = context->delay_slot ? 1 : 0;
//     write_cp0_cause(cause.value);
//     
//     // Set SR based on user/kernel mode, also set EXL bit - switching is enabled
//     struct cp0_status status;
//     read_cp0_status(status.value);
//     status.ksu = user_mode ? 0x2 : 0;
//     status.exl = 1;
//     status.erl = 0;
//     write_cp0_status(status.value);
//     
//     // Set ASID in TLB EntryHi
//     struct cp0_entry_hi hi;
//     read_cp0_entry_hi(hi.value);
//     hi.asid = asid;
//     write_cp0_entry_hi(hi.value);
    
    // Copy context to interrupt stack
    ulong *cur_stack_top = get_per_cpu(ulong, cur_interrupt_stack_top);
    memcpy((void *)*cur_stack_top, context, sizeof(struct context));
    
//     kprintf("SVC stack @ %lx\n", *cur_stack_top);
    
    // Finally mark interrupts as enabled
    set_local_int_state(1);
    
//     if (user_mode) {
//         stop(0xbbbb);
//     }
    
    // Restore GPRs
    restore_context_gpr();
}

static int switches = 0;

void no_opt switch_context(ulong sched_id, struct context *context,
                           ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb)
{
//     kprintf("To switch context, R0: %x, PC: %x, SP: %x, CPSR: %x, ASID: %lx, user: %d, page dir: %lx, TCB @ %lx, sched: %lx\n",
//           context->r0, context->pc, context->sp, context->cpsr,
//           asid, user_mode, page_dir_pfn, tcb, sched_id);
    
    // Disable local interrupts
    disable_local_int();
    
//     kprintf("To switch, ASID: %lx, TCB @ %lx, context @ %p, page dir @ %lx, R0: %x, PC: %x, SP: %x, CPSR: %x\n",
//             asid, tcb, context, page_dir_pfn, context->r0, context->pc, context->sp, context->cpsr);
    
//     if (switches >= 3 || user_mode) {
//         while (1);
//     }
//     switches++;
    
    // Copy the context to local, thus prevent page fault upon switching page dir
    struct context *per_cpu_context = get_per_cpu(struct context, cur_context);
    memcpy(per_cpu_context, context, sizeof(struct context));
//     
//     // Load context addr to k0
//     write_k0((ulong)per_cpu_context);
    
    // Set sched id
    *get_per_cpu(ulong, cur_running_sched_id) = sched_id;
    *get_per_cpu(int, cur_in_user_mode) = user_mode;
    
    // Set software thread ID with TCB
    write_software_thread_id(tcb);
    
    *(ulong *)get_per_cpu(ulong, cur_tcb_vaddr) = tcb;
    
    // Switch page dir
    switch_page_dir(page_dir_pfn);
    
    // Do the actual switch
    switch_to(per_cpu_context, user_mode, asid);
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
}
