#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "common/include/reg.h"
#include "hal/include/debug.h"
#include "hal/include/print.h"
#include "hal/include/int.h"
#include "hal/include/vecnum.h"


struct generic_timer_phys_ctrl_reg {
    union {
        u32 value;
        struct {
            u32 enabled     : 1;
            u32 masked      : 1;
            u32 asserted    : 1;
            u32 reserved    : 29;
        };
    };
};


static u32 timer_step = 0;


static int int_handler_local_timer(struct int_context *context, struct kernel_dispatch_info *kdi)
{
//     kprintf("Timer!\n");
    
    struct generic_timer_phys_ctrl_reg ctrl;
    
    // Mask the interrupt
    ctrl.value = 0;
    ctrl.enabled = 1;
    ctrl.masked = 1;
    write_generic_timer_phys_ctrl(ctrl.value);
    
    // Set a new value for compare
    write_generic_timer_phys_interval(timer_step);
    
    // Re-enable the interrupt
    ctrl.masked = 0;
    write_generic_timer_phys_ctrl(ctrl.value);
    
    // Can be taken over
    return INT_HANDLE_TYPE_TAKEOVER;
}

int is_generic_timer_asserted()
{
    struct generic_timer_phys_ctrl_reg ctrl;
    read_generic_timer_phys_ctrl(ctrl.value);
    
    return ctrl.asserted;
}

void start_generic_timer()
{
    struct generic_timer_phys_ctrl_reg ctrl;
    ctrl.value = 0;
    ctrl.enabled = 1;
    
    write_generic_timer_phys_interval(timer_step);
    write_generic_timer_phys_ctrl(ctrl.value);
}

void init_generic_timer()
{
    u32 freq = 0;
    read_generic_timer_freq(freq);
    assert(freq);
    
    // 10 times per second
    timer_step = freq / 100;
    
    // Register the handler
    set_int_vector(INT_VECTOR_LOCAL_TIMER, int_handler_local_timer);
    
    kprintf("Timer freq @ %uHz, step set @ %u\n", freq, timer_step);
}

void init_generic_timer_mp()
{
    u32 freq = 0;
    read_generic_timer_freq(freq);
    assert(freq);
    
    // 10 times per second
    timer_step = freq / 100;
    
    kprintf("Timer freq @ %uHz, step set @ %u\n", freq, timer_step);
}
