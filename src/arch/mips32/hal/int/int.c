#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/kernel.h"
#include "hal/include/mem.h"
#include "hal/include/periph.h"
#include "hal/include/int.h"


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
    u32 sr = 0;
    
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    
    // Clear global interrupt enable bit
    sr &= ~0x1;
    
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        :
        : "r" (sr)
    );
    
    int enabled = get_local_int_state();
    set_local_int_state(0);
    
    return enabled;
}

void enable_local_int()
{
    u32 sr = 0;
    
    set_local_int_state(1);
    
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    
    // Set global interrupt enable bit
    sr |= 0x1;
    
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        :
        : "r" (sr)
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

void init_int_state()
{
    set_local_int_state(0);
}


/*
 * Default dummy handler: don't do anything
 */
static int int_handler_dummy(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Interrupt, vector: %x, err_code: %x, pc: %x, sp: %x, delay slot: %x\n",
            context->vector, context->error_code,
            context->context->pc, context->context->sp, context->context->delay_slot);
    
    return 0;
}


/*
 * TLB refill handlers
 */
void tlb_refill_handler(struct context *context)
{
    // Get the bad address
    u32 bad_addr = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $8;"
        : "=r" (bad_addr)
        :
    );
    
//     kprintf("TLB refill @ %x ... ", bad_addr);
    
    // Get kernel/user mode
    int user_mode = *get_per_cpu(int, cur_in_user_mode);
    
//     kprintf("%s ... ", user_mode ? "user" : "kernel");
    
    // Try refilling TLB
    int invalid = 0;
    if (user_mode) {
        invalid = tlb_refill_user(bad_addr);
    } else {
        invalid = tlb_refill_kernel(bad_addr);
    }
    
//     kprintf("done\n");
    
    // Invalid addr
    if (invalid) {
        panic("Invalid TLB miss addr @ %x, is user mode: %d\n", bad_addr, user_mode);
    }
}

static int int_handler_tlb_refill(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    tlb_refill_handler(context->context);
    return 0;
}


/*
 * Cache error handler
 */
void cache_error_handler(struct context *context)
{
    panic("Toddler doesn't support cache error handling!!\n");
    while (1);
}


/*
 * General handler
 */
void general_except_handler(struct context *context)
{
//     kprintf("General exception!\n");
    
    // Check who is causing this interrupt
    u32 cause = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $13;"
        : "=r" (cause)
        :
    );
    
    u32 except_code = (cause >> 2) & 0x1F;
//     kprintf("\tException code: %d\n", except_code);
    
    // Figure out the internal vector number
    u32 vector = INT_VECTOR_DUMMY;
    
    if (except_code) {
        // Exception - vector number is except code
        vector = except_code;
    } else {
        // Interrupt
        if (cause & 0x40000000) {
            vector = INT_VECTOR_LOCAL_TIMER;
        }
        
        // External
        else if (cause & 0x400) {
            vector = INT_VECTOR_EXTERNAL_BASE + i8259_read_irq();
        }
    }
    
    // Tell the user
    if (vector != INT_VECTOR_LOCAL_TIMER && vector != INT_VECTOR_SYSCALL && vector != 36) {
        // Get the bad address
        u32 bad_addr = 0;
        __asm__ __volatile__ (
            "mfc0   %0, $8;"
            : "=r" (bad_addr)
            :
        );
        
        // Get the bad pc
        u32 epc = 0;
        __asm__ __volatile__ (
            "mfc0   %0, $14;"
            : "=r" (epc)
            :
        );
        u32 bad_instr_prev = *(u32 *)(epc - 4);
        u32 bad_instr = *(u32 *)epc;
        u32 bad_instr_next = *(u32 *)(epc + 4);
        
        // Delay slot
        u32 delay_slot = cause >> 31;
        
        kprintf("General exception: %d, Vector: %d, Bad PC @ %x (%x %x %x), SP @ %x, Addr @ %x, Delay slot: %x\n", except_code, vector, epc, bad_instr_prev, bad_instr, bad_instr_next, context->sp, bad_addr, delay_slot);
    }
    
    // Get the actual interrupt handler
    int_handler handler = handler = int_handler_list[vector];
    if (NULL == (void *)handler) {
        handler = int_handler_dummy;
    }
    
    // Call the real handler, the return value indicates if we need to call kernel
    struct int_context intc;
    intc.vector = vector;
    intc.error_code = except_code;
    intc.context = context;
    
    struct kernel_dispatch_info kdispatch;
    kdispatch.context = context;
    kdispatch.dispatch_type = kdisp_unknown;
    kdispatch.syscall.num = 0;
    
    // Save extra context
    save_context(context);
    
    // Call the handler!
    int call_kernel = handler(&intc, &kdispatch);
    
//     kprintf("PC: %x, SP: %x\n", context->pc, context->sp);
    
//     kprintf("call kernel: %d\n", call_kernel);
    
    // Note that if kernel is invoked, it will call sched, then never goes back to this int handler
    if (call_kernel) {
        // First of all duplicate context since it may get overwritten by a TLB miss handler
//         kprintf("duplicate context\n");
        struct context dup_ctxt = *context;
        kdispatch.context = &dup_ctxt;
//         kprintf("duplicate context done, dispatch: %x, num: %x\n", kdispatch.dispatch_type, kdispatch.syscall.num);
        
        // Set ASID to 0 - kernel
        __asm__ __volatile__ (
            "mtc0   $0, $10;"   // hi
            :
            :
        );
        
        // Clear EXL bit - so we'll get primariy correct TLB misses
        u32 sr = 0;
        __asm__ __volatile__ (
            "mfc0   %0, $12;"   // hi
            : "=r" (sr)
            :
        );
        sr &= ~0x18;    // kernel mode
        sr &= ~0x2;
        __asm__ __volatile__ (
            "mtc0   %0, $12;"   // hi
            :
            : "r" (sr)
        );
        
//         kprintf("here\n");
        
        // Tell HAL we are in kernel
        *get_per_cpu(int, cur_in_user_mode) = 0;
        
        // Go to kernel!
        kernel_dispatch(&kdispatch);
    }
    
    panic("Need to implement lazy scheduling!");
}


/*
 * Initialization
 */
void init_int()
{
    u32 ebase = 0;
    u32 sr = 0;
    struct saved_context *ctxt = get_per_cpu(struct saved_context, cur_context);
    u32 tlb_refill_stack_top = get_my_cpu_area_start_vaddr() + PER_CPU_TLB_REFILL_STACK_TOP_OFFSET;
    u32 kernel_stack_top = get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET;
    
//     u32 srsctl = 0;
    
    // Obtain old SR
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    
    // Set BEV to 0 -> use our custom handlers
    sr &= ~0x400000;
    
    // Update SR
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        "nop;"
        :
        : "r" (sr)
    );
    
    // Obtain the old ebase
    __asm__ __volatile__ (
        "mfc0   %0, $15, 1;"
        : "=r" (ebase)
        :
    );
    
    // Clear and set exception base
    ebase &= ~0x3FFFF000;
    ebase |= ((u32)&int_entry_wrapper_begin) & ~0xFFF;
    
    // Update ebase
    __asm__ __volatile__ (
        "mtc0   %0, $15, 1;"
        "nop;"
        :
        : "r" (ebase)
    );
    
    // Set
    //  k0 - base addr of current context
    //  ctxt->kernel_sp - kernel stack top
    __asm__ __volatile__ (
        "move $26, %0;"   // k0 ($26) <= ctxt
        :
        : "r" ((u32)ctxt)
    );
    ctxt->tlb_refill_sp = tlb_refill_stack_top;
    ctxt->kernel_sp = kernel_stack_top;
    
    
    // Register TLB refill general handlers
    set_int_vector(INT_VECTOR_TLB_MISS_READ, int_handler_tlb_refill);
    set_int_vector(INT_VECTOR_TLB_MISS_WRITE, int_handler_tlb_refill);
    
//     // QEMU doesn't support shadow register... so we can't use it right now
//     // Obtain old SRSCtl
//     __asm__ __volatile__ (
//         "mfc0   %0, $12, 2;"
//         : "=r" (srsctl)
//         :
//     );
//     
//     // Make sure there is at least 1 set of shadow registers
//     if (!(srsctl & 0x3C000000)) {
//         panic("Need at least 1 set of shadow registers");
//     }
//     
//     // Set ESS to 1
//     srsctl &= ~0xF000;
//     srsctl |= 0x1000;
//     
//     // Update SRSCtl
//     __asm__ __volatile__ (
//         "mtc0   %0, $12, 2;"
//         "nop;"
//         :
//         : "r" (srsctl)
//     );
    
    kprintf("Interrupt base updated, Wrapper @ %x, SR: %x, EBase: %x, Context @ %x, Kernel stack @ %x\n",
            (u32)&int_entry_wrapper_begin, sr, ebase, ctxt, kernel_stack_top);
    
//     // Test our handler
//     volatile u32 *bad_addr = (u32 *)0x4096;
//     u32 bad_value = *bad_addr;
//     kprintf("Bad value: %x\n", bad_value);
}
