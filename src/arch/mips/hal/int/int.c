#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/reg.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/kernel.h"
#include "hal/include/mem.h"
#include "hal/include/periph.h"
#include "hal/include/int.h"


ulong per_cpu_context_ptr_base = 0;

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
    struct cp0_status sr;
    
    read_cp0_status(sr.value);
    sr.ie = 0;
    write_cp0_status(sr.value);
    
    int enabled = get_local_int_state();
    set_local_int_state(0);
    
    return enabled;
}

void enable_local_int()
{
    struct cp0_status sr;
    
    set_local_int_state(1);
    
    read_cp0_status(sr.value);
    sr.ie = 1;
    write_cp0_status(sr.value);
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
    kprintf("Interrupt, vector: %p, err_code: %p, pc: %p, sp: %p, delay slot: %p\n",
            (void *)(ulong)context->vector, (void *)(ulong)context->error_code,
            (void *)(ulong)context->context->pc, (void *)(ulong)context->context->sp,
            (void *)(ulong)context->context->delay_slot);
    
    panic("Unregistered interrupt @ %p", (void *)(ulong)context->vector);
    return INT_HANDLE_TYPE_HAL;
}


/*
 * TLB refill handlers
 */
void tlb_refill_handler(struct context *context)
{
    // Get the bad address
    ulong bad_addr = 0;
    read_cp0_bad_vaddr(bad_addr);
    
    save_context(context);
    
//     kprintf("TLB refill @ %lx, PC: %lx\n", bad_addr, context->pc);
    
    // Get kernel/user mode
    int user_mode = *get_per_cpu(int, cur_in_user_mode);
    
//     kprintf("refill start\n");
    
    // Try refilling TLB
    int invalid = 0;
    if (user_mode) {
        invalid = tlb_refill_user(bad_addr);
    } else {
        invalid = tlb_refill_kernel(bad_addr);
    }
    
//     kprintf("refill done\n");
    
    // Reload TCB to k1
    ulong tcb = *(ulong *)get_per_cpu(ulong, cur_tcb_vaddr);
    write_k1(tcb);
    
//     kprintf("int done\n");
    
    // Invalid addr
    if (invalid) {
        panic("Invalid TLB miss addr @ %p, is user mode: %d\n", (void *)bad_addr, user_mode);
    }
}

static int int_handler_tlb_refill(struct int_context *context, struct kernel_dispatch_info *kdi)
{
//     struct cp0_entry_hi hi;
//     read_cp0_entry_hi(hi.value);
//     kprintf("ASID: %d\n", hi.asid);
//     
//     // Clear EXL bit - so we'll get primariy TLB misses
//     struct cp0_status status;
//     read_cp0_status(status.value);
//     kprintf("exl: %d, erl: %d, ksu: %d\n", status.exl, status.erl, status.ksu);
//     
//     // Bad addr
//     ulong bad_addr = 0;
//     read_cp0_bad_vaddr(bad_addr);
//     kprintf("Badr addr @ %lx\n", bad_addr);
//     
//     //panic("Should not get a secondary TLB miss!\n");
//     kprintf("Should not get a secondary TLB miss!\n");
    
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
    struct cp0_cause cause;
    read_cp0_cause(cause.value);
    
//     kprintf("TI: %d, IP: %d\n", cause.ti, cause.ip);
    
    u32 except_code = cause.exc_code;
    
    // Figure out the internal vector number
    u32 vector = INT_VECTOR_DUMMY;
    
    if (except_code) {
        // Exception - vector number is except code
        vector = except_code;
    } else {
        // Interrupt
        if (cause.ti) {
            // We do a polling for 8259 to see if there's any pending interrupts
            vector = INT_VECTOR_LOCAL_TIMER;
        }
        
        // External
        else if (cause.ip2) {
            vector = INT_VECTOR_EXTERNAL_BASE + i8259_read_irq();
        }
    }
    
//     if (vector != 0x8) {
//         kprintf("Vector: %x\n", vector);
//     }
    
    // Get the actual interrupt handler
    int_handler handler = get_int_handler(vector);
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
    
    // Call the handler
    int handle_type = handler(&intc, &kdispatch);
    
    // See if there's any pending IRQs when we don't need to switch to kernel
    if (handle_type == INT_HANDLE_TYPE_TAKEOVER) {
        //kprintf("Check pending IRQs\n");
        int pending_irq = i8259_read_irq();
        if (pending_irq != -1) {
            vector = INT_VECTOR_EXTERNAL_BASE + pending_irq;
            
            handler = get_int_handler(vector);
            if (NULL == (void *)handler) {
                handler = int_handler_dummy;
            }
            
            intc.vector = vector;
            intc.error_code = 0;
            intc.context = context;
            
            kdispatch.context = context;
            kdispatch.dispatch_type = kdisp_unknown;
            kdispatch.syscall.num = 0;
            
            handle_type = handler(&intc, &kdispatch);
        }
    }
    
    //kprintf("PC: %x, SP: %x\n", context->pc, context->sp);
    //kprintf("call kernel: %d\n", call_kernel);
    
    // Note that if kernel is invoked, it will call sched, then never goes back to this int handler
    handle_type = INT_HANDLE_TYPE_KERNEL;
    if (handle_type == INT_HANDLE_TYPE_KERNEL) {
        // First of all duplicate context since it may get overwritten by a TLB miss handler
        struct context dup_ctxt = *context;
        kdispatch.context = &dup_ctxt;
        //kprintf("duplicate context done, dispatch: %x, num: %x\n", kdispatch.dispatch_type, kdispatch.syscall.num);
        
        // Set ASID to 0 - kernel
        struct cp0_entry_hi hi;
        read_cp0_entry_hi(hi.value);
        hi.asid = 0;
        write_cp0_entry_hi(hi.value);
        
        // Clear EXL bit - so we'll get primariy TLB misses
        struct cp0_status status;
        read_cp0_status(status.value);
        status.exl = 0;
        status.erl = 0;
        status.ksu = 0;
        write_cp0_status(status.value);
        
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
#if (ARCH_WIDTH == 64)
    struct cp0_ebase64 ebase64;
#endif
    
    struct cp0_ebase ebase;
    ulong ebase_val = 0;
    struct cp0_status sr;
    
    struct saved_context *ctxt = get_per_cpu(struct saved_context, cur_context);
    ulong tlb_refill_stack_top = get_my_cpu_area_start_vaddr() + PER_CPU_TLB_REFILL_STACK_TOP_OFFSET - sizeof(struct saved_context);
    ulong kernel_stack_top = get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET - sizeof(struct saved_context);
    
    // Set BEV to 1 to enable custom exception handler location
    read_cp0_status(sr.value);
    sr.bev = 0;
    write_cp0_status(sr.value);
    
#if (ARCH_WIDTH == 64)
    // Read EBase
    read_cp0_ebase(ebase.value);
    
    // Test write gate
    ebase.write_gate = 1;
    write_cp0_ebase(ebase.value);
    read_cp0_ebase(ebase.value);
    
    if (ebase.write_gate) {
        read_cp0_ebase64(ebase64.value);
        ebase64.base = ((ulong)&int_entry_wrapper_begin) >> 12;
        write_cp0_ebase64(ebase64.value);
        read_cp0_ebase64(ebase_val);
    } else {
        ulong wrapper_addr = (ulong)&int_entry_wrapper_begin;
        assert((wrapper_addr >> 32) == 0xfffffffful);
        
        ebase.base = ((u32)wrapper_addr) >> 12;
        write_cp0_ebase(ebase.value);
        
        read_cp0_ebase(ebase_val);
        ebase_val |= 0xfffffffful << 32;
    }
#else
    // Set EBase
    read_cp0_ebase(ebase.value);
    ebase.base = ((ulong)&int_entry_wrapper_begin) >> 12;
    write_cp0_ebase(ebase.value);
    read_cp0_ebase(ebase_val);
#endif
    
    // Initialize context poniter for the assembly handler entry
    int page_count = 8 * num_cpus / PAGE_SIZE;
    if (!page_count) {
        page_count = 1;
    }
    per_cpu_context_ptr_base = palloc(page_count);
    per_cpu_context_ptr_base = PHYS_TO_KCODE(PFN_TO_ADDR(per_cpu_context_ptr_base));
    
    // Initizalize my save context
    ctxt->tlb_refill_sp = tlb_refill_stack_top;
    ctxt->kernel_sp = kernel_stack_top;
    
    // Set my context pointer
    struct saved_context **my_ctxt_ptr = (struct saved_context **)(ulong)(per_cpu_context_ptr_base + 8 * get_cpu_id());
    *my_ctxt_ptr = ctxt;
    
    kprintf("Per cpu ctxt ptr base @ %p, my ctxt ptr @ %p, ctxt @ %p, TLB stack @ %lx, kernel stack @ %lx\n",
            (void *)per_cpu_context_ptr_base, my_ctxt_ptr, ctxt, tlb_refill_stack_top, kernel_stack_top);
    
    // Register TLB refill general handlers
    set_int_vector(INT_VECTOR_TLB_MISS_READ, int_handler_tlb_refill);
    set_int_vector(INT_VECTOR_TLB_MISS_WRITE, int_handler_tlb_refill);
    
    kprintf("Interrupt base updated, Wrapper @ %p, SR: %x, EBase: %lx, Context @ %p, Kernel stack @ %lx\n",
            &int_entry_wrapper_begin, sr.value, ebase_val, ctxt, kernel_stack_top);
}
