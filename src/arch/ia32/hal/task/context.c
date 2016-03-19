#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"
#include "hal/include/task.h"


dec_per_cpu(ulong, cur_running_sched_id);

dec_per_cpu(int, cur_in_user_mode);
dec_per_cpu(struct context, cur_context);


void asmlinkage init_thread_context(struct context *context, ulong entry, ulong stack_top, int user_mode)
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

void asmlinkage save_context(struct context *context)
{
    //kprintf("Saving context\n");
    
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
        
        // Reload CR3
        "movl   %%eax, %%cr3;"
        "jmp    _skip_switch_page_dir;"
        
        // Done
        "_skip_switch_page_dir:"
        :
        : "a" (page_dir_pfn << 12)
    );
}

static void no_opt switch_to_user(struct context *context)
{
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
        
        "xchgw %%bx, %%bx;"
        
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
    memcpy(context, dest, sizeof(struct context) - 8);
    
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

void asmlinkage no_opt switch_context(ulong sched_id, struct context *context, ulong page_dir_pfn, int user_mode, ulong asid)
{
    //kprintf("Switching, sched_id: %p, context: %p, page dir PFN: %p, user: %d, ASID: %p\n",
    //        sched_id, context, page_dir_pfn, user_mode, asid);
    
    // Set sched id
    *get_per_cpu(ulong, cur_running_sched_id) = sched_id;
    *get_per_cpu(int, cur_in_user_mode) = user_mode;
    
    // Switch page dir
    switch_page_dir(page_dir_pfn);
    
    // Renable timer
    //start_lapic_timer();
    
    if (user_mode) {
        switch_to_user(context);
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
