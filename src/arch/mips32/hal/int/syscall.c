#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/syscall.h"
#include "hal/include/print.h"
#include "hal/include/int.h"


static int int_handler_syscall(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong num = context->context->a0;
    ulong param0 = context->context->a1;
    ulong param1 = context->context->a2;
    ulong param2 = context->context->a3;
    
    ulong ret_addr = 0;
    ulong ret_size = 0;
    int succeed = 0;
    
    int call_kernel = 1;
    
    // See if we can quickly handle this syscall in HAL
    if (num == SYSCALL_PING) {
        ret_addr = param0 + 1;
        succeed = 1;
        call_kernel = 0;
    }
    
    // Prepare to call kernel
    if (call_kernel) {
        struct kernel_dispatch_info kdispatch;
        kdispatch.context = context->context;
        kdispatch.dispatch_type = kdisp_syscall;
        kdispatch.syscall.num = num;
        kdispatch.syscall.param0 = param0;
        kdispatch.syscall.param1 = param1;
        kdispatch.syscall.param2 = param2;
        
        //kprintf("Syscall from user!\n");
    }
    
    // Prepare return value
    else {
        context->context->v0 = succeed;
        context->context->a0 = ret_addr;
        context->context->a1 = ret_size;
    }
    
    return call_kernel;
}

void init_syscall()
{
    set_int_vector(INT_VECTOR_SYSCALL, int_handler_syscall);
}

void set_syscall_return(struct context *context, int succeed, ulong return0, ulong return1)
{
    context->v0 = succeed;
    context->a0 = return0;
    context->a1 = return1;
}
