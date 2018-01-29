#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/context.h"
#include "hal/include/print.h"
#include "hal/include/string.h"
#include "hal/include/debug.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/percpu.h"
#include "hal/include/kernel.h"
#include "hal/include/int.h"
#include "hal/include/vector.h"
#include "hal/include/vecnum.h"


/*
 * Context saving
 */
#define PER_CPU_CONTEXT_OFFSET  (PER_CPU_STACK_TOP_OFFSET - sizeof(struct context))


/*
 * Start working
 */
volatile int work_started = 0;


/*
 * MSR helpers
 */
static void set_msr_bit(int bit)
{
    ulong msr = 0;
    
    __asm__ __volatile__
    (
        "sync;"
        "mfmsr %[reg];"
        : [reg]"=r"(msr)
        :
    );
    
    msr |= 0x1 << bit;
    
    __asm__ __volatile__
    (
        "mtmsr %[reg];"
        "sync;"
        :
        : [reg]"r"(msr)
    );
}

static void clear_msr_bit(int bit)
{
    ulong msr = 0;
    
    __asm__ __volatile__
    (
        "sync;"
        "mfmsr %[reg];"
        : [reg]"=r"(msr)
        :
    );
    
    msr &= ~(0x1 << bit);
    
    __asm__ __volatile__
    (
        "mtmsr %[reg];"
        "sync;"
        :
        : [reg]"r"(msr)
    );
}


/*
 * LOCAL interrupts
 */
static dec_per_cpu(int, interrupt_enabled);

int get_local_int_state()
{
    volatile int *ptr = get_per_cpu(int, interrupt_enabled);
    int enabled = *ptr;
    
    return enabled;
}

void set_local_int_state(int enabled)
{
    volatile int *ptr = get_per_cpu(int, interrupt_enabled);
    *ptr = enabled;
}

int disable_local_int()
{
    int enabled = get_local_int_state();
    clear_msr_bit(15);
    set_local_int_state(0);
    
    return enabled;
}

void enable_local_int()
{
    set_local_int_state(1);
    set_msr_bit(15);
}

void restore_local_int(int enabled)
{
    int cur_state = get_local_int_state();
    
    if (cur_state) {
        assert(enabled);
    }
    
    else if (enabled) {
        enable_local_int();
    }
}


/*
 * Start working
 */
void start_working()
{
    work_started = 1;
    
    __asm__ __volatile__
    (
        "sync;"
    );
    
    enable_local_int();
}



/*
 * Interrupt handlers
 */
static int int_handler_dummy(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Interrupt, vector: %x, err_code: %p, pc: %p, sp: %p\n",
            (int)context->vector, (void *)context->error_code,
            (void *)(ulong)context->context->pc, (void *)(ulong)context->context->r1);
    
    panic("Unregistered interrupt @ %x", (uint)context->vector);
    return INT_HANDLE_TYPE_HAL;
}

void general_except_handler()
{
    ulong vector = 0;
    struct context *context = NULL;
    
    // Mark local interrupt as disabled
    set_local_int_state(0);
    
    // Get vector and contet
    __asm__ __volatile__
    (
        "mfsprg2 %[vec];"
        : [vec]"=r"(vector)
        :
    );
    context = (struct context *)(get_my_cpu_area_start_vaddr() + PER_CPU_CONTEXT_OFFSET);
    
//     kprintf("Vector: %p, Context @ %p, PC: %p, SP: %p, SRR1: %p\n",
//         (void *)vector, context,
//         (void *)(ulong)context->pc, (void *)(ulong)context->r1,
//         (void *)(ulong)context->msr
//     );
    
    // Get handler
    int_handler handler = get_int_handler(vector);
    if (NULL == (void *)handler) {
        handler = int_handler_dummy;
    }
    
    // Call the handler, the return value indicates if we need to call kernel
    struct int_context intc;
    intc.vector = vector;
    intc.error_code = 0;
    intc.context = context;
    
    struct kernel_dispatch_info kdispatch;
    kdispatch.context = context;
    kdispatch.dispatch_type = kdisp_unknown;
    kdispatch.syscall.num = 0;
    
    // Call the handler
    int handle_type = handler(&intc, &kdispatch);
    
    // Note that if kernel is invoked, it will call sched, then never goes back to this int handler
    handle_type = INT_HANDLE_TYPE_KERNEL;
    if (handle_type == INT_HANDLE_TYPE_KERNEL) {
        // Tell HAL we are in kernel
        *get_per_cpu(int, cur_in_user_mode) = 0;
        
        // Go to kernel!
        kernel_dispatch(&kdispatch);
    }
}


/*
 * Initialization
 */
void init_int_mp()
{
    // Set up exception handler context saving frame
    ulong exp_context_frame = get_my_cpu_area_start_vaddr() + PER_CPU_CONTEXT_OFFSET;
    
    __asm__ __volatile__
    (
        "mtsprg0 %[frame];"
        :
        : [frame]"r"(exp_context_frame)
    );
    
    kprintf("\tPer-CPU exception context frame set to translated @ %p\n",
            (void *)exp_context_frame);
}

void init_int()
{
    ulong len = (ulong)&__int_entry_wrapper_end - (ulong)&__int_entry_wrapper_begin;
    
    kprintf("Initializing interrupt handlers\n");
    
    // Copy the code
    memcpy((void *)0, &__int_entry_wrapper_begin, len);
    kprintf("\tHandler template copied @ %p -> %p, size: %p\n",
            &__int_entry_wrapper_begin, (void *)0, (void *)len);
    
    // Set MSR
    clear_msr_bit(6);
    
    init_int_mp();
}

