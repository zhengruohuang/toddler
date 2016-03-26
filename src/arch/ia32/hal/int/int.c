#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/kernel.h"
#include "hal/include/int.h"


int_handler int_handler_list[IDT_ENTRY_COUNT];

static dec_per_cpu(int, interrupt_enabled);


/*
 * LOCAL interrupts
 */
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

int asmlinkage disable_local_int()
{
    __asm__ __volatile__
    (
        "cli;"
        :
        :
    );
    
    int enabled = get_local_int_state();
    set_local_int_state(0);
    
    return enabled;
}

void asmlinkage restore_local_int(int enabled)
{
    int cur_state = get_local_int_state();
    
    if (cur_state) {
        assert(enabled);
    }
    
    else if (enabled) {
        __asm__ __volatile__
        (
            "sti;"
            :
            :
        );
        
        set_local_int_state(1);
    }
}

void asmlinkage enable_local_int()
{
    __asm__ __volatile__
    (
        "sti;"
        :
        :
    );
    
    set_local_int_state(1);
}


/*
 * Initialize interrupt state
 */
void init_int_state_mp()
{
    set_local_int_state(0);
}

void init_int_state()
{
    set_local_int_state(0);
}


/*
 * Initialize int handlers
 */
void init_int_handlers()
{
    u32 i;
    
    kprintf("Initializing interrupt handlers ... ");
    
    for (i = 0; i < IDT_ENTRY_COUNT; i++) {
        int_handler_list[i] = NULL;
    }
    
    kprintf("Done!\n");
}

/*
 * General interrupt handler entry
 */
int asmlinkage int_handler_entry(u32 vector_num, u32 error_code)
{
    // For debugging, only allow one int handler at a time in a mp system
//     do {
//         if (hal_atomic_cas(&interrupt_lock, 0, 1)) {
//             break;
//         }
//     } while (1);
    
    // Get context
    struct context *context = get_per_cpu(struct context, cur_context);
    
    //kprintf("Interrupt, vector: %x, err_code: %x, eip: %x, esp: %x, cs: %x, ds: %x, es: %x, fs: %x, ss: %x eflags: %x\n",
    //        vector_num, error_code, context->eip, context->esp, context->cs, context->ds, context->es, context->fs, context->ss, context->eflags);
    
    // Get the actual interrupt handler
    int_handler handler = int_handler_list[vector_num];
    if (NULL == (void *)handler) {
        handler = int_handler_dummy;
    }
    
    // Call the real handler, the return value indicates if we need to call kernel
    struct int_context intc;
    intc.vector = vector_num;
    intc.error_code = error_code;
    intc.context = context;
    
    struct kernel_dispatch_info kdispatch;
    kdispatch.context = context;
    
    int call_kernel = handler(&intc, &kdispatch);
    
    // Note that if kernel is invoked, it will call sched, then never goes back to this int handler
    if (call_kernel) {
        //kprintf("\tSwitch to kernel\n");
        
        kernel_dispatch(&kdispatch);
    }
    
    panic("Need to implement lazy scheduling!");
    
    set_local_int_state(1);
    
    return 0;
}

/*
 * Default dummy handler: don't do anything
 */
int int_handler_dummy(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Interrupt, vector: %x, err_code: %x, eip: %x, esp: %x, cs: %x, ds: %x, es: %x, fs: %x, ss: %x eflags: %x\n",
            context->vector, context->error_code,
            context->context->eip, context->context->esp, context->context->cs, context->context->ds, context->context->es, context->context->fs, context->context->ss, context->context->eflags);
            
    return 0;
}

/*
 * Exception
 */
int int_handler_exception(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    return 1;
}

/*
 * Device
 */
int int_handler_device(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    return 1;
}
/******************************************************************************/
