#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/time.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/apic.h"


#define COUNTER_CALIBRATE_TEST_COUNT    5
#define COUNTER_CALIBRATE_MS            10


static int timer_vector = -1;

static ulong timer_counter = 0;
static int sched_freq = 1;
static ulong interrupt_counter = 0;


static int lapic_timer_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    lapic_eoi();
    return 1;
}

void init_lapic_timer_mp()
{
    stop_lapic_timer();
    
    /*
     * Initialize Timer Divide Configuration Register
     */
    struct apic_divide_config_register tdcr;
    
    tdcr.value = lapic_vaddr[APIC_TDCR];
    tdcr.div_value = APIC_DIVIDE_16;
    lapic_vaddr[APIC_TDCR] = tdcr.value;
    
    /*
     * Program local timer
     */
    struct apic_lvt_timer_register tm;
    
    tm.value = lapic_vaddr[APIC_LVT_TIME];
    tm.vector = timer_vector;
    tm.mode = APIC_TIMER_ONESHOT;
    tm.masked = 1;
    lapic_vaddr[APIC_LVT_TIME] = tm.value;
}

void init_lapic_timer()
{
    /*
     * Make sure the timer is stopped
     */
    stop_lapic_timer();
    
    /*
     * Initialize Timer Divide Configuration Register
     */
    struct apic_divide_config_register tdcr;
    
    tdcr.value = lapic_vaddr[APIC_TDCR];
    tdcr.div_value = APIC_DIVIDE_16;
    lapic_vaddr[APIC_TDCR] = tdcr.value;
    
    /*
     * Program local timer
     */
    struct apic_lvt_timer_register tm;
    
    tm.value = lapic_vaddr[APIC_LVT_TIME];
    timer_vector = alloc_int_vector(lapic_timer_handler);
        
    tm.vector = timer_vector;
    tm.mode = APIC_TIMER_ONESHOT;
    tm.masked = 1;
    lapic_vaddr[APIC_LVT_TIME] = tm.value;
    
    /*
     * Calculate timer counter
     */
    int i;
    for (i = 0; i < COUNTER_CALIBRATE_TEST_COUNT + 1; i++) {
        // Initialize as One-Shot mode
        u32 one_t1, one_t2;
        
        // Read current value of Current Count Register
        one_t1 = lapic_vaddr[APIC_CCRT];
        
        // Start the counter
        lapic_vaddr[APIC_ICRT] = 0xffffffff;
        
        // Wait until the timer starts
        do {
            if (lapic_vaddr[APIC_CCRT] != one_t1) {
                break;
            }
        } while (1);
        
        // Get current value of the timer
        one_t1 = lapic_vaddr[APIC_CCRT];
        
        // Wait for some time
        blocked_delay(COUNTER_CALIBRATE_MS);
        
        // Get the value of the timer
        one_t2 = lapic_vaddr[APIC_CCRT];
        
        // Save the test result
        // Note that the first test is the Junk Test
        if (i) {
            timer_counter += one_t1 - one_t2;
        }
    }
    
    // Get the final counter
    timer_counter /= COUNTER_CALIBRATE_TEST_COUNT;
    
    /*
     * Calculate the interrupt counter
     */
    assert(sched_freq <= 1000);
    interrupt_counter = 1000 * 1000 / sched_freq / COUNTER_CALIBRATE_MS * timer_counter;
    interrupt_counter /= 1000;

}

void start_lapic_timer()
{
    struct apic_lvt_timer_register tm;
        
    // Unmask the interrupt
    tm.value = lapic_vaddr[APIC_LVT_TIME];
    tm.mode = APIC_TIMER_PERIODIC;
    tm.masked = 0;
    lapic_vaddr[APIC_LVT_TIME] = tm.value;
    
    // Start timer
    lapic_vaddr[APIC_ICRT] = interrupt_counter;
    
    do {
        //hal_printf("Counter %d, Current %d\n", counter, lapic_vaddr[APIC_CCRT]);
        if (lapic_vaddr[APIC_CCRT] != interrupt_counter) {
            break;
        }
    } while (1);
    
    //kprintf("Timer started\n");
}

void stop_lapic_timer()
{
    struct apic_lvt_timer_register tm;
    
    // Mask the interrupt
    tm.value = lapic_vaddr[APIC_LVT_TIME];
    tm.masked = 1;
    lapic_vaddr[APIC_LVT_TIME] = tm.value;
    
    // Set the counter zero
    lapic_vaddr[APIC_ICRT] = 0;
}
