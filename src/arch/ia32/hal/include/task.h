#ifndef __ARCH_IA32_HAL_INCLUDE_TASK__
#define __ARCH_IA32_HAL_INCLUDE_TASK__


#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/proc.h"
#include "hal/include/cpu.h"


struct tss {
    u32     backlink;
    u32     esp0;                           /* stack pointer to use during interrupt */
    u32     ss0;                            /* stack segment to use during interrupt */
    u32     esp1;
    u32     ss1;
    u32     esp2;
    u32     ss2;
    u32     cr3;
    u32     eip;
    u32     flags;
    u32     eax;
    u32     ecx;
    u32     edx;
    u32     ebx;
    u32     esp;
    u32     ebp;
    u32     esi;
    u32     edi;
    u32     es;
    u32     cs;
    u32     ss;
    u32     ds;
    u32     fs;
    u32     gs;
    u32     ldt;
    u16     trap;
    u16     iopb_addr;
    /* Note that if the base address of IOPB is
     * greater than or equal to the limit(size) of
     * the TSS segment (normally it would be 104),
     * it means that there's no IOPB */
} packedstruct;


/*
 * TSS
 */
ext_per_cpu(u32, tss_user_base);
ext_per_cpu(u32, tss_user_limit);

ext_per_cpu(u32, tss_iopb_base);
ext_per_cpu(u32, tss_iopb_limit);


extern void load_tss();
extern void init_tss_mp();
extern void init_tss();


/*
 * Context
 */
ext_per_cpu(ulong, cur_running_sched_id);

ext_per_cpu(int, cur_in_user_mode);
ext_per_cpu(struct context, cur_context);

extern void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode);
extern void set_thread_context_param(struct context *context, ulong param);
extern u32 asmlinkage save_context(struct context *context);
extern void switch_context(ulong sched_id, struct context *context, ulong page_dir_pfn, int user_mode, ulong asid, struct thread_control_block *tcb);

extern void init_context_mp();
extern void init_context();


#endif
