#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "hal/include/print.h"
#include "hal/include/int.h"


static u32 timer_step = 0;


static void update_compare()
{
    u32 count = 0;
    
    __asm__ __volatile__ (
        "mfc0   %0, $9;"
        : "=r" (count)
        :
    );
    
    count += timer_step;
//     kprintf("New count: %x\n", count);
    
    __asm__ __volatile__ (
        "mtc0   %0, $11;"
        :
        : "r" (count)
    );
}

int int_handler_local_timer(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Timer!\n");
    
    // Set a new value for compare
    update_compare();
    
    // Can be taken over
    return INT_HANDLE_TYPE_TAKEOVER;
}

void enable_local_timer_interrupt()
{
    // Find out where to find the enable bit for timer
    u32 intctl = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $12, 1;"
        : "=r" (intctl)
        :
    );
    u32 ipti = (intctl >> 29) & 0x7;
    
    // Set the enable bit for our timer
    u32 sr = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    
    u32 mask = 0x1 << (8 + ipti);
    sr |= mask;
//     kprintf("SR: %x\n", sr);
    
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        :
        : "r" (sr)
    );
    
    // Set a new value for compare
    update_compare();
}

void init_local_timer()
{
    // 1G / 10 / 2 = 1,000,000,000 / 10 / 2 = 0x2FAF080
    timer_step = 0x2FAF080 / 10;
    
    set_int_vector(INT_VECTOR_LOCAL_TIMER, int_handler_local_timer);
}
