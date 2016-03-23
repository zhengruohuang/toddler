#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
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

void asmlinkage sysenter_handler_entry()
{
    ulong cr3 = 0;
    
    // Save old CR3
    __asm__ __volatile__
    (
        "movl   %%cr3, %%ebx;"
        : "=b" (cr3)
        :
    );
    
    // Switch to kernel AS
    __asm__ __volatile__
    (
        "movl   %%eax, %%cr3;"
        "jmp    _cr3_switched_to_kernel;"
        "_cr3_switched_to_kernel:"
        "nop;"
        :
        : "a" (KERNEL_PDE_PFN << 12)
    );
    
    //kprintf("Syscall from user!\n");
    
    // Switch back to user AS
    __asm__ __volatile__
    (
        "movl   %%eax, %%cr3;"
        "jmp    _cr3_switched_to_user;"
        "_cr3_switched_to_user:"
        "nop;"
        :
        : "a" (cr3)
    );
}
