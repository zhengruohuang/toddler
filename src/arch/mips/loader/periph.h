#ifndef __ARCH_MIPS32_LOADER_PERIPH_HH__
#define __ARCH_MIPS32_LOADER_PERIPH_HH__


#include "common/include/data.h"
#include "common/include/memlayout.h"


/*
 * Argument
 */
typedef __builtin_va_list   va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)


/*
 * UART
 *  UART0 @ 0x180003f8
 *  UART2 @ 0x1f000900
 */
#define UART_BASE_ADDR          (SOUTH_BRIDGE_BASE_ADDR + 0x3f8)
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
extern char uart_read();
extern void uart_write(char ch);

extern void lprintf(char *fmt, ...);

extern void scan_area(ulong start, ulong len);

extern void init_periph();


#endif
