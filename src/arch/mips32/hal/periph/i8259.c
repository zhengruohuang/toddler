#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"


static u8 master_mask = 0xff;
static u8 slave_mask = 0xff;


void i8259_enable_irq(int irq)
{
    if (irq >= 8) {
        slave_mask &= ~(0x1 << (irq - 8));
        io_write8(I8259_SLAVE_MASK_ADDR, slave_mask);
    } else {
        master_mask = ~(0x1 << irq);
        io_write8(I8259_MASTER_MASK_ADDR, master_mask);
    }
}

void i8259_disable_irq(int irq)
{
    if (irq >= 8) {
        slave_mask |= 0x1 << (irq - 8);
        io_write8(I8259_SLAVE_MASK_ADDR, slave_mask);
    } else {
        master_mask |= 0x1 << irq;
        io_write8(I8259_MASTER_MASK_ADDR, master_mask);
    }
}

void i8259_enable_irq_all()
{
    io_write8(I8259_MASTER_MASK_ADDR, 0);
    io_write8(I8259_SLAVE_MASK_ADDR, 0);
    
    master_mask = slave_mask = 0x0;
}

void i8259_disable_irq_all()
{
    io_write8(I8259_MASTER_MASK_ADDR, 0xff);
    io_write8(I8259_SLAVE_MASK_ADDR, 0xff);
    
    master_mask = slave_mask = 0xff;
}

int i8259_read_irq()
{
    int i;
    int isr = 0;
    int irq = -1;
    
    // Enter poll mode, with the following read as INT ack
    io_write8(I8259_MASTER_CMD_ADDR, 0xc);
    io_read8(I8259_MASTER_CMD_ADDR);
    
    // Read master ISR
    io_write8(I8259_MASTER_CMD_ADDR, 0x0b);
    isr = io_read8(I8259_MASTER_CMD_ADDR);
    
    if (isr == 2) {
        // Read slave ISR
        io_write8(I8259_SLAVE_CMD_ADDR, 0x0b);
        isr = 8 + io_read8(I8259_SLAVE_CMD_ADDR);
    }
    
    // Get IRQ from ISR (ISR is a bit mask)
    for (i = 0; i < 16; i++) {
        if (isr & (0x1 << i)) {
            irq = i;
            break;
        }
    }

//     kprintf("IRQ: %d, ISR: %d", irq, isr);
    
    // Exit poll mode
    io_write8(I8259_MASTER_CMD_ADDR, 0x8);
    io_read8(I8259_MASTER_CMD_ADDR);
    
    return irq;
}

void i8259_eoi(int irq)
{
//     i8259_disable_irq(irq);
    
    if (irq >= 8) {
        io_write8(I8259_SLAVE_CMD_ADDR, 0x60 + (irq - 8));
        io_write8(I8259_MASTER_CMD_ADDR, 0x62);
    } else {
        io_write8(I8259_MASTER_CMD_ADDR, 0x60 + irq);
    }
    
    io_write8(I8259_MASTER_CMD_ADDR, 0x20);
    
//     i8259_enable_irq(irq);
}


void i8259_start()
{
    // Interval timer
    i8259_enable_irq(0);
    
    // UART 0
    i8259_enable_irq(0x4);
}

void i8259_disable()
{
    i8259_disable_irq_all();
}


void init_i8259a()
{
    io_write8(I8259_MASTER_CMD_ADDR, 0x11);     // Master 8259, ICW1
    io_write8(I8259_SLAVE_CMD_ADDR, 0x11);      // Slave  8259, ICW1
    io_write8(I8259_MASTER_MASK_ADDR, 0x0);     // Master 8259, ICW2, set the initial vector of Master 8259 to 0x0.
    io_write8(I8259_SLAVE_MASK_ADDR, 0x8);      // Slave  8259, ICW2, set the initial vector of Slave 8259 to 0x8.
    io_write8(I8259_MASTER_MASK_ADDR, 0x4);     // Master 8259, ICW3, IR2 to Slave 8259
    io_write8(I8259_SLAVE_MASK_ADDR, 0x2);      // Slave  8259, ICW3, the counterpart to IR2 or Master 8259
    io_write8(I8259_MASTER_MASK_ADDR, 0x0);     // Master 8259, ICW4
    io_write8(I8259_SLAVE_MASK_ADDR, 0x0);      // Slave  8259, ICW4
    
    // Initially disabled
    i8259_disable_irq_all();
}
