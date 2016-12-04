#ifndef __ARCH_IA32_HAL_INCLUDE_TIME__
#define __ARCH_IA32_HAL_INCLUDE_TIME__


#include "common/include/data.h"
#include "hal/include/int.h"


/*
 * RTC
 */
#define RTC_REGISTER_ADDRESS   0X70
#define RTC_REGISTER_DATA      0X71

struct rtc_date_time {
    int     second;
    int     minute;
    int     hour;
    int     weekday;
    int     day;
    int     month;
    int     year;
    int     century;
};

extern void read_rtc(struct rtc_date_time *dt);
extern void init_rtc();


/*
 * PIT - i8253
 */
#define PIT_CMD_PORT    0x43
#define PIT_CH0_PORT    0x40

#define PIT_INPUT_FREQ      (1193182)

#define PIT_DELAY_TIMEOUT   (119318200)
#define PIT_DELAY_THRESHOLD (5000)
#define PIT_DELAY_MAX_MS    (50)

#define PIT_CMD_LATCH       0
#define PIT_CMD_LSB         1
#define PIT_CMD_MSB         2
#define PIT_CMD_ALL         3

#define PIT_MODE_TERMINAL       0
#define PIT_MODE_RETRIGGERABLE  1
#define PIT_MODE_RATE           2
#define PIT_MODE_SQUARE         3
#define PIT_MODE_SOFT_STROBE    4
#define PIT_MODE_HARD_STROBE    5

struct pit_cmd {
    union {
        u8 value;
        struct {
            u8 bcd      : 1;
            u8 mode     : 3;
            u8 command  : 2;
            u8 channel  : 2;
        };
    };
} packetstruct;

extern void disable_pit();
extern void pit_delay(int ms);
extern void pit_gen_tick(int freq);


/*
 * Tick
 */
#define TICK_FREQ   1
#define BLOCKED_DELAY_TEST_SEC  0

extern void change_tick(int freq);
extern void init_tick();
extern void blocked_delay(int ms);
extern void init_blocked_delay();


/*
 * System time
 */
extern void get_system_time(unsigned long *high, unsigned long *low);
extern int time_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi);
extern void init_time();


#endif
