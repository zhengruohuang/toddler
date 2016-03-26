#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "common/include/syscall.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/kernel.h"
#include "hal/include/syscall.h"


u32 syscall_proxy_entry = 0;


static void init_syscall_msr()
{
    u64 sysenter_cs = (u64)GDT_SELECTOR_CODE_K;
    u64 sysenter_esp = (u64)get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET;
    u64 sysenter_eip = (u64)(ulong)&sysenter_handler;
    
    // Initialize Sysenter parameters
    msr_write(SYSENTER_MSR_CS, &sysenter_cs);
    msr_write(SYSENTER_MSR_ESP, &sysenter_esp);
    msr_write(SYSENTER_MSR_EIP, &sysenter_eip);
}


void init_syscall_mp()
{
    kprintf("Initializing syscall\n");
    init_syscall_msr();
}

void init_syscall()
{
    kprintf("Initializing syscall\n");
    
    syscall_proxy_entry = SYSCALL_PROXY_VADDR;
    
    memcpy(
        sysenter_proxy_start_origin, (void *)SYSCALL_PROXY_VADDR,
        (ulong)sysenter_proxy_end_origin - (ulong)sysenter_proxy_start_origin
    );
    
    init_syscall_msr();
}

void asmlinkage save_context_sysenter(struct context *context)
{
    // Setup context
    context->esp = context->ecx;
    context->eip = context->edx;
    
    context->ss = GDT_SELECTOR_DATA_U;
    context->cs = GDT_SELECTOR_CODE_U;
    
    context->eflags = 0x200202;
    
    // Make sure we are in user mode
    int user_mode = *get_per_cpu(int, cur_in_user_mode);
    assert(user_mode);
    
    // Call the regular context save function
    save_context(context);
}

void asmlinkage sysenter_handler_entry(struct context *context)
{
    ulong num = context->esi;
    ulong param = context->edi;
    
    ulong ret_addr = 0;
    ulong ret_size = 0;
    int succeed = 0;
    
    int call_kernel = 1;
    
    if (num == SYSCALL_PING) {
        ret_addr = param + 1;
        succeed = 1;
        call_kernel = 0;
    }
    
    if (call_kernel) {
        struct kernel_dispatch_info kdispatch;
        kdispatch.context = context;
        kdispatch.dispatch_type = kdisp_syscall;
        kdispatch.syscall.num = num;
        kdispatch.syscall.param = param;
        kernel_dispatch(&kdispatch);
        
        //kprintf("Syscall from user!\n");
    }
    
    else {
        // Prepare return value
        context->eax = succeed;
        context->esi = ret_addr;
        context->edi = ret_size;
    }
}
