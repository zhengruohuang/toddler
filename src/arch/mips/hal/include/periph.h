#ifndef __ARCH_MIPS32_HAL_INCLUDE_PERIPH__
#define __ARCH_MIPS32_HAL_INCLUDE_PERIPH__


#include "common/include/data.h"
#include "common/include/memlayout.h"


/*
 * Print
 */
extern void draw_char(char ch);
extern void init_video();


// /*
//  * PIIX4 south bridge
//  */
// #define SOUTH_BRIDGE_BASE_ADDR  0x18000000


/*
 * Serial ports
 *  UART0 @ 0x180003f8
 *  UART2 @ 0x1f000900
 */
#define UART_BASE_ADDR          (SOUTH_BRIDGE_BASE_ADDR + 0x3f8)
#define UART_DATA_ADDR          (UART_BASE_ADDR + 0x0)
#define UART_INT_ENABLE_ADDR    (UART_BASE_ADDR + 0x1)
#define UART_INT_ID_FIFO_ADDR   (UART_BASE_ADDR + 0x2)
#define UART_LINE_CTRL_ADDR     (UART_BASE_ADDR + 0x3)
#define UART_MODEM_CTRL_ADDR    (UART_BASE_ADDR + 0x4)
#define UART_LINE_STAT_ADDR     (UART_BASE_ADDR + 0x5)
#define UART_MODEM_STAT_ADDR    (UART_BASE_ADDR + 0x6)
#define UART_SCRATCH_ADDR       (UART_BASE_ADDR + 0x7)

#define UART_DIV_LSB_ADDR       (UART_BASE_ADDR + 0x0)
#define UART_DIV_MSB_ADDR       (UART_BASE_ADDR + 0x1)

#define UART_MAX_BAUD           1152000

extern void uart_init();
extern u32 uart_read();
extern void uart_write(u32);


/*
 * Interrupt controller - 8259
 */
#define I8259_MASTER_BASE_ADDR  (SOUTH_BRIDGE_BASE_ADDR + 0x20)
#define I8259_MASTER_CMD_ADDR   (I8259_MASTER_BASE_ADDR + 0x0)
#define I8259_MASTER_MASK_ADDR  (I8259_MASTER_BASE_ADDR + 0x1)

#define I8259_SLAVE_BASE_ADDR   (SOUTH_BRIDGE_BASE_ADDR + 0xa0)
#define I8259_SLAVE_CMD_ADDR    (I8259_SLAVE_BASE_ADDR + 0x0)
#define I8259_SLAVE_MASK_ADDR   (I8259_SLAVE_BASE_ADDR + 0x1)

extern void i8259_enable_irq(int irq);
extern void i8259_disable_irq(int irq);
extern void i8259_enable_irq_all();
extern void i8259_disable_irq_all();
extern int i8259_read_irq();
extern void i8259_eoi(int irq);
extern void i8259_start();
extern void i8259_disable();
extern void init_i8259a();


/*
 * Interval timer - 8253
 */
#define I8253_BASE_ADDR         (SOUTH_BRIDGE_BASE_ADDR + 0x40)
#define I8253_CMD_ADDR          (I8253_BASE_ADDR + 0x3)
#define I8253_CH0_ADDR          (I8253_BASE_ADDR + 0x0)
#define I8253_CH1_ADDR          (I8253_BASE_ADDR + 0x1)
#define I8253_CH2_ADDR          (I8253_BASE_ADDR + 0x2)

#define I8253_INPUT_FREQ        (1193182)

#define I8253_DELAY_TIMEOUT     (119318200)
#define I8253_DELAY_THRESHOLD   (5000)
#define I8253_DELAY_MAX_MS      (50)

#define I8253_CMD_LATCH         0
#define I8253_CMD_LSB           1
#define I8253_CMD_MSB           2
#define I8253_CMD_ALL           3

#define I8253_MODE_TERMINAL         0
#define I8253_MODE_RETRIGGERABLE    1
#define I8253_MODE_RATE             2
#define I8253_MODE_SQUARE           3
#define I8253_MODE_SOFT_STROBE      4
#define I8253_MODE_HARD_STROBE      5

struct i8253_cmd {
    union {
#if ARCH_LITTLE_ENDIAN
        struct {
            u8 bcd      : 1;
            u8 mode     : 3;
            u8 command  : 2;
            u8 channel  : 2;
        };
#else
        struct {
            u8 channel  : 2;
            u8 command  : 2;
            u8 mode     : 3;
            u8 bcd      : 1;
        };
#endif
        
        u8 value;
    };
} packetstruct;

extern void pit_disable();
extern void pit_delay(int ms);
extern void pit_gen_tick(int freq);


/*
 * RTC
 */
#define RTC_BASE_ADDR           (SOUTH_BRIDGE_BASE_ADDR + 0x70)
#define RTC_CMD_ADDR            (RTC_BASE_ADDR + 0x0)
#define RTC_DATA_ADDR           (RTC_BASE_ADDR + 0x1)

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
 * General
 */
extern u32 io_read32(u32 addr);
extern void io_write32(u32 addr, u32 val);
extern u8 io_read8(u32 addr);
extern void io_write8(u32 addr, u8 val);

extern void init_periph();


#endif
