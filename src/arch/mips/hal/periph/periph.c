#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/periph.h"


u32 io_read32(u32 addr)
{
    volatile u32 *ptr = (u32 *)(SEG_LOW_DIRECT + addr);
    return *ptr;
}

void io_write32(u32 addr, u32 val)
{
    volatile u32 *ptr = (u32 *)(SEG_LOW_DIRECT + addr);
    *ptr = val;
}

u8 io_read8(u32 addr)
{
    volatile u8 *ptr = (u8 *)(SEG_LOW_DIRECT + addr);
    return *ptr;
}

void io_write8(u32 addr, u8 val)
{
    volatile u8 *ptr = (u8 *)(SEG_LOW_DIRECT + addr);
    *ptr = val;
}


/*
 * Initialization
 */
void init_periph()
{

}
