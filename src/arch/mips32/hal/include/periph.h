#ifndef __ARCH_MIPS32_HAL_INCLUDE_PERIPH__
#define __ARCH_MIPS32_HAL_INCLUDE_PERIPH__


#include "common/include/data.h"


/*
 * UART
 *  UART0 @ 0x180003f8
 *  UART2 @ 0x1f000900
 */
#define UART_BASE_ADDR          0x180003f8
#define UART_DATA_ADDR          (UART_BASE_ADDR + 0x0)
#define UART_INT_ENABLE_ADDR    (UART_BASE_ADDR + 0x8)
#define UART_INT_ID_FIFO_ADDR   (UART_BASE_ADDR + 0x10)
#define UART_LINE_CTRL_ADDR     (UART_BASE_ADDR + 0x18)
#define UART_MODEM_CTRL_ADDR    (UART_BASE_ADDR + 0x20)
#define UART_LINE_STAT_ADDR     (UART_BASE_ADDR + 0x28)
#define UART_MODEM_STAT_ADDR    (UART_BASE_ADDR + 0x30)
#define UART_SCRATCH_ADDR       (UART_BASE_ADDR + 0x38)

#define UART_DIV_LSB_ADDR       (UART_BASE_ADDR + 0x0)
#define UART_DIV_MSB_ADDR       (UART_BASE_ADDR + 0x8)

#define UART_MAX_BAUD           1152000


/*
 * Functions
 */
extern void uart_init();
extern u32 uart_read();
extern void uart_write(u32);

extern void init_periph();


#endif
