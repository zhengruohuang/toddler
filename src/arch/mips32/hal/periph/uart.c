#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/periph.h"


/*
 * UART
 */
void uart_init()
{
    u32 data = 0;   // 8-bit data
    u32 parity = 0; // No parity
    u32 stop = 0;   // 1-bit stop
    
    u32 baud = 9600;
    u32 divisor = UART_MAX_BAUD / baud;
    
    // Disable interrupts
    io_write32(UART_INT_ENABLE_ADDR, 0);

    // Set up buad rate
    io_write32(UART_LINE_CTRL_ADDR, 0x80);
    io_write32(UART_DIV_LSB_ADDR, divisor & 0xff);
    io_write32(UART_DIV_MSB_ADDR, (divisor & 0xff00)>>8);
    io_write32(UART_LINE_CTRL_ADDR, 0x0);

    // Set data format
    io_write32(UART_LINE_CTRL_ADDR, data | parity | stop);
}

u32 uart_read()
{
    u32 ready = 0;
    while (!ready) {
        ready = io_read32(UART_LINE_STAT_ADDR) & 0x1;
    }
    
    return io_read32(UART_DATA_ADDR);
}

void uart_write(u32 val)
{
    u32 ready = 0;
    while (!ready) {
        ready = io_read32(UART_LINE_STAT_ADDR) & 0x20;
    }
    
    io_write32(UART_DATA_ADDR, val & 0xff);
}
