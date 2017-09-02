#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/vector.h"
#include "hal/include/int.h"
#include "hal/include/vecnum.h"


#define TICK_FREQ               20


static ulong decrementer_value = 0;


static void restart_decrementer()
{
    __asm__ __volatile__
    (
        "mtdec %[dec];"
        :
        : [dec]"r"(decrementer_value)
    );
}

static ulong read_decrementer()
{
    ulong dec = 0;
    
    __asm__ __volatile__
    (
        "mfdec %[dec];"
        : [dec]"=r"(dec)
        :
    );
    
    return dec;
}

static int int_handler_decrementer(struct int_context *context, struct kernel_dispatch_info *kdi)
{
//     kprintf("Timer @ %p!\n",(void *)read_decrementer());
    
    // Restart the decrementer
    restart_decrementer();
    
    // Can be taken over
    return INT_HANDLE_TYPE_TAKEOVER;
}

void init_decrementer()
{
    decrementer_value = cpu_info.freq_base / TICK_FREQ;
    kprintf("Decrementer set to %p, tick frequency: %d\n", (void *)decrementer_value, TICK_FREQ);
    
    set_int_vector(INT_VECTOR_DECREMENTER, int_handler_decrementer);
    restart_decrementer();
}
