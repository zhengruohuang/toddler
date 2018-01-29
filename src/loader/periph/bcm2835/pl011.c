#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


#define PL011_BASE              (0x201000ul)

#define PL011_FR_BUSY           ((u32)(1 << 3))     // Set to 1 when UART is transmitting data
#define PL011_FR_RXFE           ((u32)(1 << 4))     // Set to 1 when RX FIFO/register is empty
#define PL011_FR_TXFF           ((u32)(1 << 5))     // Set to 1 when TX FIFO/register is full
#define PL011_FR_RXFF           ((u32)(1 << 6))     // Set to 1 when RX FIFO/register is full
#define PL011_FR_TXFE           ((u32)(1<< 7))      // Set to 1 when TX FIFO/register is empty

#define PL011_BAUD_INT(x)       (3000000 / (16 * (x)))
#define PL011_BAUD_FRAC(x)      (int)((((3000000.0 / (16.0 * (x))) - PL011_BAUD_INT(x)) * 64.0) + 0.5)

#define PL011_LCRH_BRK          ((u32)(1 << 0))    // Send break
#define PL011_LCRH_PEN          ((u32)(1 << 1))    // Parity enable
#define PL011_LCRH_EPS          ((u32)(1 << 2))    // Even parity select
#define PL011_LCRH_STP2         ((u32)(1 << 3))    // Two stop bits select
#define PL011_LCRH_FEN          ((u32)(1 << 4))    // Enable FIFOs
#define PL011_LCRH_WLEN8        ((u32)(0x03<<5))   // Word length 8 bits
#define PL011_LCRH_WLEN7        ((u32)(0x02<<5))   // Word length 7 bits
#define PL011_LCRH_WLEN6        ((u32)(0x01<<5))   // Word length 6 bits
#define PL011_LCRH_WLEN5        ((u32)(0x00<<5))   // Word length 5 bits
#define PL011_LCRH_SPS          ((u32)(1 << 7))    // Sticky parity select



struct bcm2835_pl011 {
    volatile u32 DR;            // Data Register
    volatile u32 RSRECR;        // Receive status register/error clear register
    volatile u32 PAD[4];        // Padding
    volatile u32 FR;            // Flag register
    volatile u32 RES1;          // Reserved
    volatile u32 ILPR;          // Not in use
    volatile u32 IBRD;          // Integer Baud rate divisor
    volatile u32 FBRD;          // Fractional Baud rate divisor
    volatile u32 LCRH;          // Line Control register
    volatile u32 CR;            // Control register
    volatile u32 IFLS;          // Interupt FIFO Level Select Register
    volatile u32 IMSC;          // Interupt Mask Set Clear Register
    volatile u32 RIS;           // Raw Interupt Status Register
    volatile u32 MIS;           // Masked Interupt Status Register
    volatile u32 ICR;           // Interupt Clear Register
    volatile u32 DMACR;         // DMA Control Register
};


static volatile struct bcm2835_pl011 *pl011;

static u32 pl011_baud_rate = 115200;


void init_bcm2835_pl011(ulong bcm2835_base)
{
    pl011 = (void *)(bcm2835_base + PL011_BASE);
    
    // Disable everything
    pl011->CR = 0;

    // Tell GPIO to enable PL011
    bcm2835_gpio_enable_pl011();

    // Poll the "flags register" to wait for the UART to stop transmitting or receiving
    while (pl011->FR & PL011_FR_BUSY);

    // Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
    pl011->LCRH &= ~PL011_LCRH_FEN;

    // Clear all interrupt status
    pl011->ICR = 0x7FF;
    
    // IBRD = UART_CLK / (16 * BAUD_RATE)
    // FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
    // UART_CLK = 3000000
    // BAUD_RATE = 115200
    pl011->IBRD = PL011_BAUD_INT(pl011_baud_rate);
    pl011->FBRD = PL011_BAUD_FRAC(pl011_baud_rate);
    
    // Set N, 8, 1, FIFO disabled
    pl011->LCRH = PL011_LCRH_WLEN8;
    
    // Enable UART
    pl011->CR = 0x301;
}

u8 bcm2835_pl011_read()
{
//     // Wait until the UART has an empty space in the FIFO
//     while ((uart->MU_LSR & UART1_LSR_DR) == 0);
// 
//     // Write the character to the FIFO for transmission
//     return (u8)uart->MU_IO;
    return 0;
}


void bcm2835_pl011_write(u8 ch)
{
    // Wait until the UART has an empty space in the FIFO
    while (pl011->FR & 0x20);

    // Write the character to the FIFO for transmission
    pl011->DR = ch;
}
