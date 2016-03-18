#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/task.h"


void init_thread_context(struct context *context, ulong entry, ulong stack_top, int user_mode)
{
    // Segmentation registers
    if (user_mode) {
        context->cs = GDT_SELECTOR_CODE_U;
        context->ds = GDT_SELECTOR_DATA_U;
        context->es = GDT_SELECTOR_DATA_U;
        context->fs = GDT_SELECTOR_DATA_U;
        context->ss = GDT_SELECTOR_DATA_U;
        
        context->eflags = 0x202;
    }
    
    else {
        context->cs = GDT_SELECTOR_CODE_K;
        context->ds = GDT_SELECTOR_DATA_K;
        context->es = GDT_SELECTOR_DATA_K;
        context->fs = GDT_SELECTOR_DATA_K;
        context->ss = GDT_SELECTOR_DATA_K;
        
        context->eflags = 0x282;
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

int asmlinkage save_context(ulong cur_stack_ptr)
{
    return 0;
}

void switch_context()
{
}
