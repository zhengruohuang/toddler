#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/lib.h"
#include "hal/include/int.h"
#include "hal/include/periph.h"


/*
 * Read and write
 */
u32 uart_read()
{
    u32 ready = 0;
    while (!ready) {
        ready = io_read8(UART_LINE_STAT_ADDR) & 0x1;
    }
    
    return io_read8(UART_DATA_ADDR);
}

void uart_write(u32 val)
{
    u32 ready = 0;
    while (!ready) {
        ready = io_read8(UART_LINE_STAT_ADDR) & 0x20;
    }
    
    io_write8(UART_DATA_ADDR, val & 0xff);
}

/*
 * Interrupt handler
 */
static int int_handler_uart(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong count = 0;
    ulong data = 0;
    ulong byte = 0;
    
    io_write8(UART_INT_ENABLE_ADDR, 0x0);
    
    while (io_read8(UART_LINE_STAT_ADDR) & 0x1) {
        byte = (ulong)io_read8(UART_DATA_ADDR);
        switch (byte) {
            case 0x7f:
                byte = '\b';
                break;
            case '\r':
                byte = '\n';
                break;
            default:
                break;
        }
        
        data <<= 8;
        data |= byte;
        count++;
    }
    
    if (count > sizeof(ulong)) {
        count = sizeof(ulong);
    }
    
    // Clear buffer
//     io_write8(UART_INT_ENABLE_ADDR, 0x0);
//     io_write8(UART_INT_ID_FIFO_ADDR, 0x6);
    
    kdi->dispatch_type = kdisp_interrupt;
    kdi->interrupt.irq = 1;
    kdi->interrupt.vector = context->vector;
    
    kdi->interrupt.param0 = (ulong)count;
    kdi->interrupt.param1 = data;
    kdi->interrupt.param2 = 0;
    
    i8259_eoi(0x4);
    io_write8(UART_INT_ENABLE_ADDR, 0x1);
    
//     kprintf("UART: %x, count: %d\n", data, count);
    
    return INT_HANDLE_TYPE_KERNEL;
}

/*
 * UART
 */
void uart_init()
{
    io_write8(UART_INT_ENABLE_ADDR, 0x0);   // Disable interrupts
    io_write8(UART_INT_ID_FIFO_ADDR, 0x6);  // Disable FIFO
    io_write8(UART_LINE_CTRL_ADDR, 0x80);   // Enable DLAB (set baud rate divisor)
    io_write8(UART_DIV_LSB_ADDR, 0x3);      // Set divisor to 3 (lo byte) 38400 baud
    io_write8(UART_DIV_MSB_ADDR, 0x0);      //                  (hi byte)
    io_write8(UART_LINE_CTRL_ADDR, 0x3);    // 8 bits, no parity, one stop bit
//     io_write8(UART_INT_ID_FIFO_ADDR, 0x7);  // Enable FIFO, clear them, with 1-byte threshold
    io_write8(UART_MODEM_CTRL_ADDR, 0xb);     // IRQs enabled, RTS/DSR set
    
    // Enable interrupts
    set_int_vector(INT_VECTOR_EXTERNAL_BASE + 4, int_handler_uart);
    io_write8(UART_INT_ENABLE_ADDR, 0x1);
    
//     io_read8(UART_DATA_ADDR);
}
