#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/debug.h"
#include "hal/include/vector.h"
#include "hal/include/periph.h"


/*
 * Reference
 * http://www.leidinger.net/FreeBSD/dox/dev_uart/html/d2/d9a/uart__dev__z8530_8c_source.html
 * https://github.com/lattera/freebsd/blob/master/sys/dev/ic/z8530.h
 * https://github.com/nccgroup/TriforceAFL/blob/master/qemu_mode/qemu/roms/openbios/drivers/escc.c
 * https://stuff.mit.edu/afs/sipb/contrib/doc/specs/protocol/chrp/chrp_io.pdf
 */

// Serial registers
#define ESCC_REG_CTRL    0
#define ESCC_REG_DATA    1

// Tx Empty
#define ESCC_RX_AVAIL    0x1
#define ESCC_BES_TXE     0x4

/* Write registers. */
#define	WR_CR		0	/* Command Register. */
#define	WR_IDT		1	/* Interrupt and Data Transfer Mode. */
#define	WR_IV		2	/* Interrupt Vector (shared). */
#define	WR_RPC		3	/* Receive Parameters and Control. */
#define	WR_MPM		4	/* Miscellaneous Parameters and Modes. */
#define	WR_TPC		5	/* Transmit Parameters and Control. */
#define	WR_SCAF		6	/* Sync Character or (SDLC) Address Field. */
#define	WR_SCF		7	/* Sync Character or (SDCL) Flag. */
#define	WR_EFC		7	/* Extended Feature and FIFO Control. */
#define	WR_TB		8	/* Transmit Buffer. */
#define	WR_MIC		9	/* Master Interrupt Control (shared). */
#define	WR_MCB1		10	/* Miscellaneous Control Bits (part 1 :-). */
#define	WR_CMC		11	/* Clock Mode Control. */
#define	WR_TCL		12	/* BRG Time Constant Low. */
#define	WR_TCH		13	/* BRG Time Constant High. */
#define	WR_MCB2		14	/* Miscellaneous Control Bits (part 2 :-). */
#define	WR_IC		15	/* Interrupt Control. */

/* Interrupt Control (WR15). */
#define	IC_BRK		0x80	/* Break (Abort) IE. */
#define	IC_TXU		0x40	/* Tx Underrun IE. */
#define	IC_CTS		0x20	/* CTS IE. */
#define	IC_SYNC		0x10	/* Sync IE. */
#define	IC_DCD		0x08	/* DCD IE. */
#define	IC_FIFO		0x04	/* SDLC FIFO Enable. */
#define	IC_ZC		0x02	/* Zero Count IE. */
#define	IC_EF		0x01	/* Extended Feature Enable. */

/* Interrupt and Data Transfer Mode (WR1). */
#define	IDT_WRE		0x80	/* Wait/DMA Request Enable. */
#define	IDT_REQ		0x40	/* DMA Request. */
#define	IDT_WRR		0x20	/* Wait/DMA Reuest on Receive. */
#define	IDT_RISC	0x18	/* Rx Int. on Special Condition Only. */
#define	IDT_RIA		0x10	/* Rx Int. on All Characters. */
#define	IDT_RIF		0x08	/* Rx Int. on First Character. */
#define	IDT_PSC		0x04	/* Parity is Special Condition. */
#define	IDT_TIE		0x02	/* Tx Int. Enable. */
#define	IDT_XIE		0x01	/* Ext. Int. Enable. */

/* Master Interrupt Control (WR9). */
#define	MIC_FHR		0xc0	/* Force Hardware Reset. */
#define	MIC_CRA		0x80	/* Channel Reset A. */
#define	MIC_CRB		0x40	/* Channel Reset B. */
#define	MIC_SIE		0x20	/* Software INTACK Enable. */
#define	MIC_SH		0x10	/* Status High. */
#define	MIC_MIE		0x08	/* Master Interrupt Enable. */
#define	MIC_DLC		0x04	/* Disable Lower Chain. */
#define	MIC_NV		0x02	/* No Vector. */
#define	MIC_VIS		0x01	/* Vector Includes Status. */

/* Command Register (WR0). */
#define	CR_RSTTXU	0xc0	/* Reset Tx. Underrun/EOM. */
#define	CR_RSTTXCRC	0x80	/* Reset Tx. CRC. */
#define	CR_RSTRXCRC	0x40	/* Reset Rx. CRC. */
#define	CR_RSTIUS	0x38	/* Reset Int. Under Service. */
#define	CR_RSTERR	0x30	/* Error Reset. */
#define	CR_RSTTXI	0x28	/* Reset Tx. Int. */
#define	CR_ENARXI	0x20	/* Enable Rx. Int. */
#define	CR_ABORT	0x18	/* Send Abort. */
#define	CR_RSTXSI	0x10	/* Reset Ext/Status Int. */

// 23 for heathrow, 37 for openpic
// #define PIC_WIRED_VECTOR    37

static int escc_wired_int_vector;
static volatile u8 *escc_ctrl;
static volatile u8 *escc_data;


static void ctrl_delay()
{
    int i;
    for (i = 0; i < 1024; i++);
}

static void ctrl_write(int reg, u8 val)
{
    assert(reg < 16);
    
    // Select the register
    *escc_ctrl = reg;
    ctrl_delay();
    
    // Write the data
    *escc_ctrl = val;
    ctrl_delay();
}

static u8 ctrl_read(int reg)
{
    assert(reg < 16);
    
    // Select the register
    *escc_ctrl = reg;
    ctrl_delay();
    
    // Read the data
    return *escc_ctrl;
}

static void data_write(u8 val)
{
    *escc_data = val;
}

static u8 data_read()
{
    return *escc_data;
}

static int escc_received()
{
    return ctrl_read(ESCC_REG_CTRL) & ESCC_RX_AVAIL;
}

static u8 escc_read()
{
    while (!(ctrl_read(ESCC_REG_CTRL) & ESCC_RX_AVAIL));
    return data_read();
}

static void escc_write(u8 ch)
{
    while (!(ctrl_read(ESCC_REG_CTRL) & ESCC_BES_TXE));
    data_write(ch);
}

static int escc_int_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong count = 0;
    ulong data = 0;
    ulong byte = 0;
    
    while (escc_received()) {
        byte = (ulong)escc_read();
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
    
//     kprintf("data: %lx, count: %lx\n", data, count);
    
    kdi->dispatch_type = kdisp_interrupt;
    kdi->interrupt.irq = 1;
    kdi->interrupt.vector = context->vector;
    
    kdi->interrupt.param0 = (ulong)count;
    kdi->interrupt.param1 = data;
    kdi->interrupt.param2 = 0;
    
    // Reset int status
    ctrl_write(WR_CR, CR_RSTIUS);
    
    // EOI
    pic_eoi(escc_wired_int_vector);
    
    return INT_HANDLE_TYPE_KERNEL;
}

void escc_draw_char(char ch)
{
    escc_write(ch);
}

void start_escc()
{
//     ctrl_write(WR_MIC, MIC_NV | MIC_CRA);
    
//     ctrl_write(WR_IC, IC_BRK | IC_CTS | IC_DCD);
//     ctrl_write(WR_IDT, IDT_XIE | IDT_TIE | IDT_RIA);
    ctrl_write(WR_IDT, IDT_RIA);
    ctrl_write(WR_IV, 0);
    ctrl_write(WR_MIC, MIC_NV | MIC_MIE);
    
    // Register the handler
    pic_register_wired(escc_wired_int_vector, escc_int_handler);
}

void init_escc()
{
    struct boot_parameters *bp = get_bootparam();
    if (bp->video_mode != VIDEO_SERIAL) {
        return;
    }
    
    if (bp->has_openpic) {
        escc_wired_int_vector = 37;
    } else {
        escc_wired_int_vector = 23;
    }
    
    // We'll skip the complex initialization procedure
    // because we only use ESCC in qemu
    escc_ctrl = (void *)bp->serial_addr;
    escc_data = (void *)bp->serial_addr + 0x10;
}
