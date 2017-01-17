#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "common/include/proc.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"
#include "hal/include/task.h"


dec_per_cpu(ulong, cur_running_sched_id);

dec_per_cpu(int, cur_in_user_mode);
dec_per_cpu(struct context, cur_context);


void init_thread_context(struct context *context, ulong entry, ulong param, ulong stack_top, int user_mode)
{
    // Segmentation registers
    if (user_mode) {
        context->cs = GDT_SELECTOR_CODE_U;
        context->ds = GDT_SELECTOR_DATA_U;
        context->es = GDT_SELECTOR_DATA_U;
        context->fs = GDT_SELECTOR_DATA_U;
        context->ss = GDT_SELECTOR_DATA_U;
        
        context->eflags = 0x200202;
    }
    
    else {
        context->cs = GDT_SELECTOR_CODE_K;
        context->ds = GDT_SELECTOR_DATA_K;
        context->es = GDT_SELECTOR_DATA_K;
        context->fs = GDT_SELECTOR_DATA_K;
        context->ss = GDT_SELECTOR_DATA_K;
        
        context->eflags = 0x200202;
    }
    
    // Thread control block
    context->gs = GDT_SELECTOR_PER_CPU_U;
    
    // General registers
    context->eax = 0;
    context->ebx = 0;
    context->ecx = 0;
    context->edx = 0;
    context->esi = 0;
    context->edi = 0;
    context->ebp = 0;
    
    context->eip = entry;
    context->esp = stack_top;
}

void set_thread_context_param(struct context *context, ulong param)
{
}

u32 asmlinkage save_context(struct context *context)
{
    //kprintf("Saving context\n");
    
//     __asm__ __volatile__
//     (
//         "xchgw %%bx, %%bx;"
//         :
//         :
//     );
    
    // Set interrupt state as disabled
    set_local_int_state(0);
    
    struct context *dest = get_per_cpu(struct context, cur_context);
    int user_mode = *get_per_cpu(int, cur_in_user_mode);
    
    dest->cs = context->cs;
    dest->ds = context->ds;
    dest->es = context->es;
    dest->fs = context->fs;
    dest->gs = context->gs;
    
    dest->eflags = context->eflags;
    dest->eip = context->eip;
    
    dest->eax = context->eax;
    dest->ebx = context->ebx;
    dest->ecx = context->ecx;
    dest->edx = context->edx;
    dest->esi = context->esi;
    dest->edi = context->edi;
    dest->ebp = context->ebp;
    
    dest->vector = context->vector;
    dest->error_code = context->error_code;
    
    if (user_mode) {
        dest->esp = context->esp;
        dest->ss = context->ss;
    } else {
        //kprintf("Saving kernel...");
        dest->esp = (ulong)context + sizeof(struct context) - 8;
        dest->ss = context->ds;
    }
    
    if (user_mode) {
        return 0;
    } else {
        ulong my_cpu_area = get_my_cpu_area_start_vaddr();
        
        if ((ulong)context >= my_cpu_area) {
            return 0;
        }
        
        //kprintf("Need to switch stack for kernel->kernel\n");
        
        ulong new_stack = my_cpu_area + PER_CPU_STACK_TOP_OFFSET - sizeof(struct context);
        memcpy((void *)new_stack, context, sizeof(struct context));
        return new_stack;
    }
    
    //kprintf(" Done\n");
}

static void no_opt switch_page_dir(ulong page_dir_pfn)
{
    __asm__ __volatile__
    (
        // Check whether we need to reload CR3
        "movl   %%cr3, %%ebx;"
        "cmpl   %%eax, %%ebx;"
        
        // If there is no need ot reload CR3, directly do switch
        "jz     _skip_switch_page_dir;"
        
        //"xchgw  %%bx, %%bx;"
        
        // Reload CR3
        "movl   %%eax, %%cr3;"
        "jmp    _skip_switch_page_dir;"
        
        // Done
        "_skip_switch_page_dir:"
        
        //"nop;"
        //"movl   %%esp, %%ecx;"
        
        "nop;"
        //"xchgw  %%bx, %%bx;"
        :
        : "a" (page_dir_pfn << 12)
    );
}

static void no_opt switch_to_user(struct context *context)
{
    //kprintf("To switch to user, target ESP: %p\n", context->esp);
    
    __asm__ __volatile__
    (
        //"xchgw  %%bx, %%bx;"
        
        // Set stack top to the beginning of register struct
        "movl   %%esi, %%esp;"
        
        // Restore segmentation registers
        "popl   %%eax;"
        "movw   %%ax, %%gs;"
        "popl   %%eax;"
        "movw   %%ax, %%fs;"
        "popl   %%eax;"
        "movw   %%ax, %%es;"
        "popl   %%eax;"
        "movw   %%ax, %%ds;"
        
        // Restore GP registerss
        "popal;"
        
        // Skip vector and err code
        "addl   $8, %%esp;"
        
        //"xchgw %%bx, %%bx;"
        
        // Return to context using iretd
        "iretl;"
        :
        : "S" ((ulong)context)
    );
}

static void no_opt switch_to_kernel(struct context *context)
{
    //kprintf("To switch, target ESP: %p\n", context->esp);
    
    struct context *dest = (struct context *)(context->esp - sizeof(struct context) + 8);
    memcpy(dest, context, sizeof(struct context) - 8);
    
    __asm__ __volatile__
    (
        // Set stack top to the beginning of register struct
        "movl   %%esi, %%esp;"
        
        // Restore segmentation registers
        "popl   %%eax;"
        "movw   %%ax, %%gs;"
        "popl   %%eax;"
        "movw   %%ax, %%fs;"
        "popl   %%eax;"
        "movw   %%ax, %%es;"
        "popl   %%eax;"
        "movw   %%ax, %%ds;"
        
        // Restore GP registerss
        "popal;"
        
        // Skip vector and err code
        "addl   $8, %%esp;"
        
        //"xchgw %%bx, %%bx;"
        
        // Return to context using iretd
        "iretl;"
        :
        : "S" ((ulong)dest)
    );
}

void no_opt switch_context(ulong sched_id, struct context *context,
                                      ulong page_dir_pfn, int user_mode, ulong asid, ulong tcb)
{
    // Copy the context for user mode switch
    struct context copy_context;
    if (user_mode) {
        memcpy(&copy_context, context, sizeof(struct context));
        //kprintf("User\n");
    }
    
    // Set interrupt state as enabled
    set_local_int_state(1);
    
//     kprintf("Switching, sched_id: %p, context: %p, page dir PFN: %p, user: %d, ASID: %p\n",
//            sched_id, context, page_dir_pfn, user_mode, asid);
    
    // Set sched id
    *get_per_cpu(ulong, cur_running_sched_id) = sched_id;
    *get_per_cpu(int, cur_in_user_mode) = user_mode;
    
//     // Set TCB
//     struct thread_control_block *cur_tcb = (struct thread_control_block *)get_my_cpu_tcb_start_vaddr();
//     cur_tcb->msg_send = tcb->msg_send;
//     cur_tcb->msg_recv = tcb->msg_recv;
//     cur_tcb->tls = tcb->tls;
//     cur_tcb->proc_id = tcb->proc_id;
//     cur_tcb->thread_id = tcb->thread_id;
    
    // Reload TCB addr to GS
    gdt_reload_gs(tcb, sizeof(struct thread_control_block));
    
    // Switch page dir
    // Note that kprintf may not be useable after this
    switch_page_dir(page_dir_pfn);
    
    // Switch the context
    if (user_mode) {
        switch_to_user(&copy_context);
    } else {
        switch_to_kernel(context);
    }
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
