#include "common/include/data.h"
#include "hal/include/int.h"
#include "hal/include/periph.h"


/*
 * Start working
 */
volatile int work_started = 0;


/*
 * Start working
 */
void start_working()
{
    work_started = 1;
    
    start_periph();
    
    __asm__ __volatile__ (
        "sync;"
    );
    
    enable_local_int();
}
