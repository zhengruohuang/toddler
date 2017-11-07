#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "common/include/reg.h"
#include "hal/include/print.h"
#include "hal/include/int.h"


static u32 timer_step = 0;


static void update_compare()
{
    u32 count = 0;
    read_cp0_count(count);
    
    count += timer_step;
    write_cp0_compare(count);
    
    //kprintf("New count: %x\n", count);
}

int int_handler_local_timer(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    //kprintf("Timer!\n");
    
    // Set a new value for compare
    update_compare();
    
    // Can be taken over
    return INT_HANDLE_TYPE_TAKEOVER;
}

void enable_local_timer_interrupt()
{
    // Set a new value for compare
    update_compare();
}

void init_local_timer()
{
    // 1G / 10 / 2 = 1,000,000,000 / 10 / 2 = 0x2FAF080
    timer_step = 0x2FAF080 / 10;
    
    set_int_vector(INT_VECTOR_LOCAL_TIMER, int_handler_local_timer);
}
