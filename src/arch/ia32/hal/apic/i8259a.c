#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/i8259a.h"


void init_i8259a()
{
    io_out8(I8259A_M_CTL,       0x11);                      // Master 8259, ICW1
    io_out8(I8259A_S_CTL,       0x11);                      // Slave  8259, ICW1
    io_out8(I8259A_M_CTLMASK,   I8259A_VECTOR_IRQ0);        // Master 8259, ICW2, set the initial vector of Master 8259 to 0x20.
    io_out8(I8259A_S_CTLMASK,   I8259A_VECTOR_IRQ8);        // Slave  8259, ICW2, set the initial vector of Slave 8259 to 0x28.
    io_out8(I8259A_M_CTLMASK,   0x4);                       // Master 8259, ICW3, IR2 to Slave 8259
    io_out8(I8259A_S_CTLMASK,   0x2);                       // Slave  8259, ICW3, the counterpart to IR2 or Master 8259
    io_out8(I8259A_M_CTLMASK,   0x1);                       // Master 8259, ICW4
    io_out8(I8259A_S_CTLMASK,   0x1);                       // Slave  8259, ICW4
    
    io_out8(I8259A_M_CTLMASK,   0xFF);                      // Master 8259, OCW1
    io_out8(I8259A_S_CTLMASK,   0xFF);                      // Slave  8259, OCW1
}

void start_i8259a()
{
}

void disable_i8259a()
{
    io_out8(0xa1, 0xff);
    io_out8(0x21, 0xff);
}

void i8259a_enable_irq(int irq_num)
{
    if (irq_num > 8) {
        io_out8(I8259A_S_CTLMASK, io_in8(I8259A_S_CTLMASK) & (~(1 << (irq_num - 8))));
    } else {
        io_out8(I8259A_M_CTLMASK, io_in8(I8259A_M_CTLMASK) & (~(1 << irq_num)));
    }
}

void i8259a_disable_irq(int irq_num)
{
    if (irq_num > 8) {
        io_out8(I8259A_S_CTLMASK, io_in8(I8259A_S_CTLMASK) | (1 << (irq_num - 8)));
    } else {
        io_out8(I8259A_M_CTLMASK, io_in8(I8259A_M_CTLMASK) | (1 << irq_num));
    }
}

void i8259a_enable_irq_all()
{
    io_out8(I8259A_M_CTLMASK, 0);   // Master 8259, OCW1
    io_out8(I8259A_S_CTLMASK, 0);   // Slave  8259, OCW1
}

void i8259a_disable_irq_all()
{
    io_out8(I8259A_M_CTLMASK, 0xFF);    // Master 8259, OCW1.
    io_out8(I8259A_S_CTLMASK, 0xFF);    // Slave  8259, OCW1.
}
