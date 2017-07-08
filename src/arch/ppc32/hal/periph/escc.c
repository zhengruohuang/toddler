#include "common/include/data.h"


#define ESCC_BASE_ADDR  0x80013020

#define DEFAULT_RCLK    307200
#define UART_PCLK       0

// Serial registers
#define REG_CTRL    0
#define REG_DATA    1

// Write registers
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

/* Clock Mode Control (WR11). */
#define	CMC_XTAL	0x80	/* -RTxC connects to quartz crystal. */
#define	CMC_RC_DPLL	0x60	/* Rx Clock from DPLL. */
#define	CMC_RC_BRG	0x40	/* Rx Clock from BRG. */
#define	CMC_RC_TRXC	0x20	/* Rx Clock from -TRxC. */
#define	CMC_RC_RTXC	0x00	/* Rx Clock from -RTxC. */
#define	CMC_TC_DPLL	0x18	/* Tx Clock from DPLL */
#define	CMC_TC_BRG	0x10	/* Tx Clock from BRG */
#define	CMC_TC_TRXC	0x08	/* Tx Clock from -TRxC. */
#define	CMC_TC_RTXC	0x00	/* Tx Clock from -RTxC. */
#define	CMC_TRXC_OUT	0x04	/* -TRxC is output. */
#define	CMC_TRXC_DPLL	0x03	/* -TRxC from DPLL */
#define	CMC_TRXC_BRG	0x02	/* -TRxC from BRG */
#define	CMC_TRXC_XMIT	0x01	/* -TRxC from Tx clock. */
#define	CMC_TRXC_XTAL	0x00	/* -TRxC from XTAL. */

/* Miscellaneous Control Bits part 1 (WR10). */
#define	MCB1_CRC1	0x80	/* CRC presets to 1. */
#define	MCB1_FM0	0x60	/* FM0 Encoding. */
#define	MCB1_FM1	0x40	/* FM1 Encoding. */
#define	MCB1_NRZI	0x20	/* NRZI Encoding. */
#define	MCB1_NRZ	0x00	/* NRZ Encoding. */
#define	MCB1_AOP	0x10	/* Active On Poll. */
#define	MCB1_MI		0x08	/* Mark Idle. */
#define	MCB1_AOU	0x04	/* Abort On Underrun. */
#define	MCB1_LM		0x02	/* Loop Mode. */
#define	MCB1_SIX	0x01	/* 6 or 12 bit SYNC. */

#define BES_TXE     0x04    // Tx Empty


static void barrier()
{
}

static void reg_write(int reg, ulong val)
{
    u8 *regp = (void *)ESCC_BASE_ADDR;
    regp += reg << 4;
    
    *regp = (u8)val;
}

static u8 reg_read(int reg)
{
    u8 *regp = (void *)ESCC_BASE_ADDR;
    regp += reg << 4;
    
    u8 val = *regp;
    
    return val;
}

static void mreg_write(int reg, ulong val)
{
    reg_write(REG_CTRL, reg);
    barrier();
    reg_write(REG_CTRL, val);
}

static u8 mreg_read(int reg)
{
    reg_write(REG_CTRL, reg);
    barrier();
    return reg_read(REG_CTRL);
}


void escc_draw_char(char ch)
{
    int ready = 0;
    while (!ready) {
        ready = reg_read(REG_CTRL) & BES_TXE;
    }
    
    reg_write(REG_DATA, ch);
    barrier();
}

void init_escc()
{
    return;
    //uint8_t tpc;

    // Assume we don't need to perform a full hardware reset
    mreg_write(WR_MIC, MIC_NV | MIC_CRA);
    barrier();
    
    // Set clock sources
    mreg_write(WR_CMC, CMC_RC_BRG | CMC_TC_BRG);
    mreg_write(WR_MCB2, UART_PCLK);
    barrier();
    
    // Set data encoding
    mreg_write(WR_MCB1, MCB1_NRZ);
    barrier();

    //tpc = TPC_DTR | TPC_RTS;
}
