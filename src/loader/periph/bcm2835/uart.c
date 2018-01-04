#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


#define UART_BASE               (0x215000ul)


#define UART1_LCR_7BITS         0x02    // 7 bits mode
#define UART1_LCR_8BITS         0x03    // 8 bits mode
#define UART1_LCR_BREAK         0x40    // send break
#define UART1_LCR_DLAB          0x80    // DLAB access

#define UART1_LSR_DR            0x01    // Receive Data ready
#define UART1_LSR_OE            0x02    // Receiver overrun error
#define UART1_LSR_THRE          0x20    // Transmitter holding register
#define UART1_LSR_TEMT          0x40    // Transmitter empty

#define UART1_CNTL_REC_ENBL     0x01    // receiver enable
#define UART1_CNTL_TRN_ENBL     0x02    // transmitter enable
#define UART1_CNTL_AUTO_RTR     0x04    // RTR set by RX FF level
#define UART1_CNTL_AUTO_CTS     0x08    // CTS auto stops transmitter
#define UART1_CNTL_FLOW3        0x00    // Stop on RX FF 3 entries left
#define UART1_CNTL_FLOW2        0x10    // Stop on RX FF 2 entries left
#define UART1_CNTL_FLOW1        0x20    // Stop on RX FF 1 entries left
#define UART1_CNTL_FLOW4        0x30    // Stop on RX FF 4 entries left
#define UART1_CNTL_AURTRINV     0x40    // Invert AUTO RTR polarity
#define UART1_CNTL_AUCTSINV     0x80    // Invert AUTO CTS polarity


struct bcm2835_uart {
    volatile u32 IRQ;
    volatile u32 ENA;

    volatile u32 reserved1[14];

    volatile u32 MU_IO;
    volatile u32 MU_IER;
    volatile u32 MU_IIR;
    volatile u32 MU_LCR;
    volatile u32 MU_MCR;
    volatile u32 MU_LSR;
    volatile u32 MU_MSR;
    volatile u32 MU_SCRATCH;
    volatile u32 MU_CNTL;
    volatile u32 MU_STAT;
    volatile u32 MU_BAUD;
};


static volatile struct bcm2835_uart *uart;


void init_bcm2835_uart(ulong bcm2835_base)
{
    uart = (void *)(bcm2835_base + UART_BASE);
    
    uart->ENA = BCM2835_AUX_ENABLE_UART1;
    uart->MU_CNTL = 0x00;
    uart->MU_LCR = UART1_LCR_8BITS;
    uart->MU_MCR = 0x00;
    uart->MU_IER = 0x05;
    uart->MU_IIR = 0xC6;
    uart->MU_BAUD = 270;    // Baud rate = sysclk/(8*(BAUD_REG+1))
    
    // Tell GPIO to enable the UART
    bcm2835_gpio_enable_uart();

    // turn on the uart for send and receive
    uart->MU_CNTL = UART1_CNTL_REC_ENBL | UART1_CNTL_TRN_ENBL;
}

u8 bcm2835_uart_read()
{
    // Wait until the UART has an empty space in the FIFO
    while ((uart->MU_LSR & UART1_LSR_DR) == 0);

    // Write the character to the FIFO for transmission
    return (u8)uart->MU_IO;
}


void bcm2835_uart_write(u8 ch)
{
    // Wait until the UART has an empty space in the FIFO
    while ((uart->MU_LSR & UART1_LSR_THRE) == 0);

    // Write the character to the FIFO for transmission
    uart->MU_IO = ch;
}
