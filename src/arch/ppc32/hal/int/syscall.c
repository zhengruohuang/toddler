#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "common/include/syscall.h"
#include "hal/include/print.h"
#include "hal/include/vector.h"
#include "hal/include/int.h"
#include "hal/include/vecnum.h"


static int int_handler_syscall(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong num = context->context->r3;
    ulong param0 = context->context->r4;
    ulong param1 = context->context->r5;
    ulong param2 = context->context->r6;
    
//     kprintf("Syscall, num: %p, param0: %p, param1: %p, param2: %p\n",
//         (void *)num, (void *)param0, (void *)param1, (void *)param2
//     );
    
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
        kdi->dispatch_type = kdisp_syscall;
        kdi->syscall.num = num;
        kdi->syscall.param0 = param0;
        kdi->syscall.param1 = param1;
        kdi->syscall.param2 = param2;
        
//         kprintf("Syscall from user!\n");
    }
    
    // Prepare return value
    else {
        context->context->r3 = succeed;
        context->context->r4 = ret_addr;
        context->context->r5 = ret_size;
    }
    
    //context->context->pc += 4;
    
    return call_kernel;
}

void init_syscall()
{
    set_int_vector(INT_VECTOR_SYSCALL, int_handler_syscall);
}

void set_syscall_return(struct context *context, int succeed, ulong return0, ulong return1)
{
    context->r3 = succeed;
    context->r4 = return0;
    context->r5 = return1;
}
