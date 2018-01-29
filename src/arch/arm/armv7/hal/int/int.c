#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/context.h"
#include "common/include/reg.h"
#include "common/include/atomic.h"
#include "hal/include/print.h"
#include "hal/include/bootparam.h"
#include "hal/include/percpu.h"
#include "hal/include/cpu.h"
#include "hal/include/debug.h"
#include "hal/include/string.h"
#include "hal/include/bit.h"
#include "hal/include/cpu.h"
#include "hal/include/kernel.h"
#include "hal/include/mem.h"
#include "hal/include/vector.h"
#include "hal/include/vecnum.h"
#include "hal/include/int.h"
#include "hal/include/pic.h"
#include "hal/include/periph.h"


ulong per_cpu_context_ptr_base = 0;

dec_per_cpu(ulong, cur_interrupt_stack_top);

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

int disable_local_int()
{
    __asm__ __volatile__ (
        "cpsid aif;"
        :
        :
        : "memory"
    );
    
    int enabled = get_local_int_state();
    set_local_int_state(0);
    
    return enabled;
}

void enable_local_int()
{
    set_local_int_state(1);
    
    __asm__ __volatile__ (
        "cpsie iaf;"
        :
        :
        : "memory"
    );
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
 * Initialize interrupt state
 */
void init_int_state_mp()
{
    set_local_int_state(0);
}

// FIXME: nobody called this?
void init_int_state()
{
    set_local_int_state(0);
}


/*
 * Default dummy handler
 */
static int int_handler_dummy(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Interrupt, Vector: %lx, PC: %x, SP: %x, CPSR: %x\n",
            context->vector, context->context->pc, context->context->sp, context->context->cpsr);
    
    panic("Unregistered interrupt @ %p", (void *)(ulong)context->vector);
    
    return INT_HANDLE_TYPE_HAL;
}


/*
 * General handler
 */
struct l1table;
extern struct l1table *kernel_l1table;

void int_handler_entry(int except, struct context *context)
{
    // Switch to kernel's page table
    write_trans_tab_base0(kernel_l1table);
    inv_tlb_all();
    atomic_membar();
    
    // Mark local interrupts as disabled
    disable_local_int();
    
//     kprintf("General exception!\n");
//     kprintf("Interrupt, Except: %d, PC: %x, SP: %x, CPSR: %x\n",
//             except, context->pc, context->sp, context->cpsr);
    
    // Figure out the real vector number
    int vector = 0;
    
    switch (except) {
    // System call
    case INT_VECTOR_SVC:
        vector = INT_VECTOR_SYSCALL;
        break;
    
    // Page fault
    case INT_VECTOR_FETCH:
    case INT_VECTOR_DATA:
        break;
    
    // Interrupt
    case INT_VECTOR_IRQ:
        vector = periph_get_irq_vector();
        break;
    
    // Local timer
    case INT_VECTOR_FIQ:
        vector = periph_get_fiq_vector();
        break;
    
    // Illegal
    case INT_VECTOR_RESET:
    case INT_VECTOR_RESERVED:
        vector = INT_VECTOR_DUMMY;
        break;
    
    // Not implement yet
    case INT_VECTOR_UNDEFINED:
        vector = INT_VECTOR_DUMMY;
        break;
    
    // Unknown
    default:
        vector = INT_VECTOR_DUMMY;
        break;
    }
    
    // Get the actual interrupt handler
    int_handler handler = get_int_handler(vector);
    if (!handler) {
        handler = int_handler_dummy;
    }
    
    // Call the real handler, the return value indicates if we need to call kernel
    struct int_context intc;
    intc.vector = vector;
    intc.error_code = except;
    intc.context = context;
    
    struct kernel_dispatch_info kdispatch;
    kdispatch.context = context;
    kdispatch.dispatch_type = kdisp_unknown;
    kdispatch.syscall.num = 0;
    
    // Call the handler
    int handle_type = handler(&intc, &kdispatch);
    
    if (except == INT_VECTOR_FIQ) {
        // FIQ handler must have TAKE_OVER or HAL type
        if (handle_type != INT_HANDLE_TYPE_HAL && handle_type != INT_HANDLE_TYPE_TAKEOVER) {
            panic("FIQ handler must have TAKE_OVER or HAL type!\n");
        }
        
        // Switch to kernel if FIQ did not preempt any normal interrupts
        struct proc_status_reg status;
        status.value = context->cpsr;
        
        if (status.mode == 0x10 || status.mode == 0x1f) {
            handle_type = INT_HANDLE_TYPE_KERNEL;
        }
    }
    
    // Note that if kernel is invoked,
    // kernel will call sched and never go back to this int handler
    if (handle_type == INT_HANDLE_TYPE_KERNEL) {
        // Tell HAL we are in kernel
        *get_per_cpu(int, cur_in_user_mode) = 0;
        
        // Go to kernel!
        kernel_dispatch(&kdispatch);
    }
    
//     panic("Need to implement lazy scheduling!");
}


/*
 * Initialization
 */
void init_int()
{
    // Copy the vectors to the target address
    void *vec_target = (void *)0xffff0000ul;
    memcpy(vec_target, int_entry_wrapper_begin,
           (ulong)int_entry_wrapper_end - (ulong)int_entry_wrapper_begin);
    
    // Enable high addr vector
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.high_except_vec = 1;
    write_sys_ctrl(sys_ctrl.value);
    
    // Register the dummy handler
    set_int_vector(INT_VECTOR_DUMMY, int_handler_dummy);
    
    ulong stack_top = get_my_cpu_area_start_vaddr() +
        PER_CPU_STACK_TOP_OFFSET - sizeof(struct context);
    
    // Align the stack to 16B
    stack_top = ALIGN_DOWN(stack_top, 16);
    
    // Remember the stack top
    ulong *cur_stack_top = get_per_cpu(ulong, cur_interrupt_stack_top);
    *cur_stack_top = stack_top;
    
    kprintf("Set exception handler stack @ %lx\n", stack_top);
    
    ulong saved_sp, saved_lr;
    __asm__ __volatile__ (
        // Save current sp and lr
        "mov %[saved_sp], sp;"
        "mov %[saved_lr], lr;"
        
        // SVC
        "cpsid aif, #0x13;"
        "mov sp, %[stack];"
        
        // UNDEF
        "cpsid aif, #0x1b;"
        "mov sp, %[stack];"
        
        // ABT
        "cpsid aif, #0x17;"
        "mov sp, %[stack];"
        
        // IRQ
        "cpsid aif, #0x12;"
        "mov sp, %[stack];"
        
        // FIQ
        "cpsid aif, #0x11;"
        "mov sp, %[stack];"
        
        // Move to System mode
        "cpsid aif, #0x1f;"
        
        // Restore saved sp and lr
        "mov sp, %[saved_sp];"
        "mov lr, %[saved_lr];"
        
        : [saved_sp] "=&r" (saved_sp), [saved_lr] "=&r" (saved_lr)
        : [stack] "r" (stack_top)
    );
    
//     while (1);
}
