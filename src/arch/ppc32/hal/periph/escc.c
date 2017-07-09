#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"


// Serial registers
#define ESCC_REG_CTRL    0
#define ESCC_REG_DATA    1

// Tx Empty
#define ESCC_BES_TXE     0x04


static u8 *escc_base;


static void reg_write(int reg, ulong val)
{
    u8 *regp = escc_base;
    regp += reg << 4;
    
    *regp = (u8)val;
}

static u8 reg_read(int reg)
{
    u8 *regp = escc_base;
    regp += reg << 4;
    
    u8 val = *regp;
    
    return val;
}

void escc_draw_char(char ch)
{
    int ready = 0;
    while (!ready) {
        ready = reg_read(ESCC_REG_CTRL) & ESCC_BES_TXE;
    }
    
    reg_write(ESCC_REG_DATA, ch);
}

void init_escc()
{
    struct boot_parameters *bp = get_bootparam();
    if (bp->video_mode != VIDEO_SERIAL) {
        return;
    }
    
    // We'll skip the complex initialization procedure
    // because we only use ESCC in qemu
    escc_base = (void *)bp->serial_addr;
}
