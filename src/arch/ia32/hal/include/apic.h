#ifndef __ARCH_IA32_HAL_INCLUDE_APIC__
#define __ARCH_IA32_HAL_INCLUDE_APIC__


#include "common/include/data.h"


/*******************************************************************************
 * Common
 ******************************************************************************/
#define APIC_FIXED          (0 << 0)
#define APIC_LOPRI          (1 << 0)

#define APIC_APIC_ID_COUNT  256

/* local APIC macros */
#define APIC_IPI_INIT       0
#define APIC_IPI_STARTUP    0

/*  Delivery modes. */
#define APIC_DELMOD_FIXED   0x0U
#define APIC_DELMOD_LOWPRI  0x1U
#define APIC_DELMOD_SMI     0x2U
/* 0x3 reserved */
#define APIC_DELMOD_NMI     0x4U
#define APIC_DELMOD_INIT    0x5U
#define APIC_DELMOD_STARTUP 0x6U
#define APIC_DELMOD_EXTINT  0x7U

/*  Destination modes. */
#define APIC_DESTMOD_PHYS   0x0U
#define APIC_DESTMOD_LOGIC  0x1U

/*  Trigger Modes. */
#define APIC_TRIGMOD_EDGE   0x0U
#define APIC_TRIGMOD_LEVEL  0x1U

/*  Levels. */
#define APIC_LEVEL_DEASSERT 0x0U
#define APIC_LEVEL_ASSERT   0x1U

/*  Destination Shorthands. */
#define APIC_SHORTHAND_NONE         0x0U
#define APIC_SHORTHAND_SELF         0x1U
#define APIC_SHORTHAND_ALL_INCL     0x2U
#define APIC_SHORTHAND_ALL_EXCL     0x3U

/*  Interrupt Input Pin Polarities. */
#define APIC_POLARITY_HIGH          0x0U
#define APIC_POLARITY_LOW           0x1U

/*  Divide Values. (Bit 2 is always 0) */
#define APIC_DIVIDE_2       0x0U
#define APIC_DIVIDE_4       0x1U
#define APIC_DIVIDE_8       0x2U
#define APIC_DIVIDE_16      0x3U
#define APIC_DIVIDE_32      0x8U
#define APIC_DIVIDE_64      0x9U
#define APIC_DIVIDE_128     0xaU
#define APIC_DIVIDE_1       0xbU

/*  Timer Modes. */
#define APIC_TIMER_ONESHOT  0x0U
#define APIC_TIMER_PERIODIC 0x1U
#define APIC_TIMER_TSC      0x2U

/*  Delivery status. */
#define APIC_DELIVS_IDLE    0x0U
#define APIC_DELIVS_PENDING 0x1U

/*  Destination masks. */
#define APIC_DEST_ALL       0xffU

/*  Dest format models. */
#define APIC_MODEL_FLAT     0xfU
#define APIC_MODEL_CLUSTER  0x0U


#define APIC_IDT_ITEMS  64
#define APIC_IVT_ITEMS  APIC_IDT_ITEMS
#define APIC_IVT_FIRST  0

#define APIC_EXC_COUNT  32
#define APIC_IRQ_COUNT  16

#define APIC_IVT_EXCBASE   0
#define APIC_IVT_IRQBASE   (APIC_IVT_EXCBASE + APIC_EXC_COUNT)
#define APIC_IVT_FREEBASE  (APIC_IVT_IRQBASE + APIC_IRQ_COUNT)

#define APIC_IRQ_CLK       0
#define APIC_IRQ_KBD       1
#define APIC_IRQ_PIC1      2
#define APIC_IRQ_PIC_SPUR  7
#define APIC_IRQ_MOUSE     12
#define APIC_IRQ_NE2000    5

/* This one must have four least significant bits set to ones */
#define APIC_VECTOR_APIC_SPUR  (APIC_IVT_ITEMS - 1)

//#if (((APIC_VECTOR_APIC_SPUR + 1) % 16) || APIC_VECTOR_APIC_SPUR >= APIC_IVT_ITEMS)
//#error Wrong definition of VECTOR_APIC_SPUR
//#endif

#define APIC_VECTOR_DEBUG                   1
#define APIC_VECTOR_CLK                     (APIC_IVT_IRQBASE + APIC_IRQ_CLK)
#define APIC_VECTOR_PIC_SPUR                (APIC_IVT_IRQBASE + APIC_IRQ_PIC_SPUR)
#define APIC_VECTOR_SYSCALL                 (APIC_IVT_FREEBASE)
#define APIC_VECTOR_TLB_SHOOTDOWN_IPI       (APIC_IVT_FREEBASE + 1)
#define APIC_VECTOR_DEBUG_IPI               (APIC_IVT_FREEBASE + 2)
/******************************************************************************/


/*******************************************************************************
 *                         Interrupt Command Register                          *
 ******************************************************************************/
/*  Interrupt Command Register. */
#define APIC_ICR_LO  (0x300 / sizeof(u32))
#define APIC_ICR_HI  (0x310 / sizeof(u32))

struct apic_interupt_command_register {
    union {
        u32         value_low;
        struct {
            u32     vector          : 8;            /* < Interrupt Vector. */
            u32     delmod          : 3;            /* < Delivery Mode. */
            u32     destmod         : 1;            /* < Destination Mode. */
            u32     delivs          : 1;            /* < Delivery status (RO). */
            u32     reserved_1      : 1;            /* < Reserved. */
            u32     level           : 1;            /* < Level. */
            u32     trigger_mode    : 1;            /* < Trigger Mode. */
            u32     reserved_2      : 2;            /* < Reserved. */
            u32     shorthand       : 2;            /* < Destination Shorthand. */
            u32     reserved_3      : 12;           /* < Reserved. */
        };
    };
    union {
        u32         value_high;
        struct {
            u32     reserved        : 24;           /* < Reserved. */
            u8      dest;                           /* < Destination field. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                           Error Status Register                             *
 ******************************************************************************/
/* End Of Interrupt. */
#define APIC_EOI  (0x0b0 / sizeof(u32))

/* Error Status Register. */
#define APIC_ESR  (0x280 / sizeof(u32))

struct apic_error_status_register {
    union {
        u32     value;
        u8      err_bitmap;
        struct {
            u32     send_checksum_error             : 1;
            u32     receive_checksum_error          : 1;
            u32     send_accept_error               : 1;
            u32     receive_accept_error            : 1;
            u32     reserved_1                      : 1;
            u32     send_illegal_vector             : 1;
            u32     received_illegal_vector         : 1;
            u32     illegal_register_address        : 1;
            u32     reserved_2                      : 24;
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                           Task Priority Register                            *
 ******************************************************************************/
/* Task Priority Register */
#define APIC_TPR  (0x080 / sizeof(u32))

struct apic_task_priority_register {
    union {
        u32 value;
        struct {
            u32     pri_sc          : 4;            /* < Task Priority Sub-Class. */
            u32     pri             : 4;            /* < Task Priority. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                    Spurious-Interrupt Vector Register                       *
 ******************************************************************************/
/* Spurious-Interrupt Vector Register. */
#define APIC_SVR  (0x0f0 / sizeof(u32))

struct apic_spurious_vector_register {
    union {
        u32 value;
        struct {
            u32     vector          : 8;            /* < Spurious Vector. */
            u32     lapic_enabled   : 1;            /* < APIC Software Enable/Disable. */
            u32     focus_checking  : 1;            /* < Focus Processor Checking. */
            u32     reserved        : 22;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                       Time Divide Configuration Register                    *
 ******************************************************************************/
/* Time Divide Configuration Register. */
#define APIC_TDCR  (0x3e0 / sizeof(u32))

struct apic_divide_config_register {
    union {
        u32 value;
        struct {
            u32     div_value       : 4;            /* < Divide Value, bit 2 is always 0. */
            u32     reserved        : 28;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                              LVT Timer register                             *
 ******************************************************************************/
/* Initial Count Register for Timer */
#define APIC_ICRT           (0x380 / sizeof(u32))

/* Current Count Register for Timer */
#define APIC_CCRT           (0x390 / sizeof(u32))

/* LVT Timer register. */
#define APIC_LVT_TIME       (0x320 / sizeof(u32))

struct apic_lvt_timer_register {
    union {
        u32 value;
        struct {
            u32     vector          : 8;            /* < Local Timer Interrupt vector. */
            u32     reserved_1      : 4;            /* < Reserved. */
            u32     delivs          : 1;            /* < Delivery status (RO). */
            u32     reserved_2      : 3;            /* < Reserved. */
            u32     masked          : 1;            /* < Interrupt Mask. */
            u32     mode            : 1;            /* < Timer Mode. */
            u32     reserved_3      : 14;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                              LVT LINT registers                             *
 ******************************************************************************/
/*  LVT LINT registers. */
#define APIC_LVT_LINT0  (0x350 / sizeof(u32))
#define APIC_LVT_LINT1  (0x360 / sizeof(u32))

struct apic_lvt_lint_register {
    union {
        u32 value;
        struct {
            u32     vector          : 8;            /* < LINT Interrupt vector. */
            u32     delmod          : 3;            /* < Delivery Mode. */
            u32     reserved_1      : 1;            /* < Reserved. */
            u32     delivs          : 1;            /* < Delivery status (RO). */
            u32     intpol          : 1;            /* < Interrupt Input Pin Polarity. */
            u32     irr             : 1;            /* < Remote IRR (RO). */
            u32     trigger_mode    : 1;            /* < Trigger Mode. */
            u32     masked          : 1;            /* < Interrupt Mask. */
            u32     reserved_2      : 15;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                              LVT Error register                             *
 ******************************************************************************/
/* LVT Error register. */
#define APIC_LVT_ERR        (0x370 / sizeof(u32))

struct apic_lvt_error_register {
    union {
        u32 value;
        struct {
            u32     vector          : 8;            /* < Local Timer Interrupt vector. */
            u32     reserved_1      : 4;            /* < Reserved. */
            u32     delivs          : 1;            /* < Delivery status (RO). */
            u32     reserved_2      : 3;            /* < Reserved. */
            u32     masked          : 1;            /* < Interrupt Mask. */
            u32     reserved_3      : 15;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                           Local APIC ID Register.                           *
 ******************************************************************************/
/*  Local APIC ID Register. */
#define APIC_LAPIC_ID  (0x020 / sizeof(u32))

struct apic_lapic_id {
    union {
        u32 value;
        struct {
            u32     reserved        : 24;           /* < Reserved. */
            u32     apic_id         : 8;            /* < Local APIC ID. */
        };
    };
} packedstruct;

/*  Local APIC Version Register */
#define APIC_LAVR                   (0x030 / sizeof(u32))
#define APIC_LAVR_MASK              0xff

#define APIC_IS_LOCAL_APIC(x)       (((x) & APIC_LAVR_MASK & 0xf0) == 0x1)
#define APIC_IS_LAPIC_82489DX(x)    ((((x) & APIC_LAVR_MASK & 0xf0) == 0x0))
#define APIC_IS_LAPIC_XAPIC(x)      (((x) & APIC_LAVR_MASK) == 0x14)
/******************************************************************************/


/*******************************************************************************
 *                        Logical Destination Register                         *
 ******************************************************************************/
/*  Logical Destination Register. */
#define APIC_LDR   (0x0d0 / sizeof(u32))

struct apic_logical_destination_register {
    union {
        u32 value;
        struct {
            u32     reserved        : 24;           /* < Reserved. */
            u32     id              : 8;            /* < Logical APIC ID. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                         Destination Format Register                         *
 ******************************************************************************/
/*  Destination Format Register. */
#define APIC_DFR    (0x0e0 / sizeof(u32))

struct apic_destination_format_register {
    union {
        u32 value;
        struct {
            u32     reserved        : 28;           /* < Reserved, all ones. */
            u32     model           : 4;            /* < Model. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                                  IO APIC                                    *
 ******************************************************************************/
/* IO APIC */
#define APIC_IO_REGSEL       (0x00 / sizeof(u32))
#define APIC_IO_WIN          (0x10 / sizeof(u32))

#define APIC_IO_APICID       0x00
#define APIC_IO_APICVER      0x01
#define APIC_IO_APICARB      0x02
#define APIC_IO_REDTBL       0x10

/*  I/O Register Select Register. */
struct apic_io_regseclect_register {
    union {
        u32 value;
        struct {
            u32     reg_addr        : 8;            /* < APIC Register Address. */
            u32     reserved        : 24;           /* < Reserved. */
        };
    };
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                          I/O Redirection Register                           *
 ******************************************************************************/
/*  I/O Redirection Register. */
struct io_redirection_register {
    union {
        u32 low;
        struct {
            u32     vector          : 8;            /* < Interrupt Vector. */
            u32     delmod          : 3;            /* < Delivery Mode. */
            u32     destmod         : 1;            /* < Destination mode. */
            u32     delivs          : 1;            /* < Delivery status (RO). */
            u32     intpol          : 1;            /* < Interrupt Input Pin Polarity. */
            u32     irr             : 1;            /* < Remote IRR (RO). */
            u32     trigger_mode    : 1;            /* < Trigger Mode. */
            u32     masked          : 1;            /* < Interrupt Mask. */
            u32     reserved_1      : 15;           /* < Reserved. */
        };
    };
    
    union {
        u32 high;
        struct {
            u32     reserved_2      : 24;           /* < Reserved. */
            u32     dest            : 8;            /* < Destination Field. */
        };
    };
    
} packedstruct;
/******************************************************************************/


/*******************************************************************************
 *                       IO APIC Identification Register                       *
 ******************************************************************************/
/*  IO APIC Identification Register. */
struct apic_ioapic_id {
    union {
        u32 value;
        struct {
            u32     reserved_1      : 24;                   /* < Reserved. */
            u32     apic_id         : 4;                    /* < IO APIC ID. */
            u32     reserved_2      : 4;                    /* < Reserved. */
        } packedstruct;
    };
} packedstruct;
/******************************************************************************/



/*
 * IO APIC
 */
extern int ioapic_count;

extern void init_ioapic();


/*
 * Local APIC
 */
extern ulong lapic_paddr;

extern void lapic_eoi();
extern int get_apic_id_by_cpu_id(int cpu_id);
extern int get_cpu_id_by_apic_id(int apic_id);
extern int get_apic_id();
extern int get_cpu_id();
extern void init_lapic_mp();
extern void init_lapic();


/*
 * IPI
 */
extern int ipi_send_startup(int apicid);


/*
 * IO APIC
 */

extern void ioapic_start();
extern void ioapic_init_redirection_table(int chip, int pin, int vector, int trigger, int polarity);
extern void ioapic_change_io_redirection_table(int chip, int pin, int dest, int vec, u32 flags);
extern void ioapic_disable_irq(u16 irq);
extern void ioapic_enable_irq(u16 irq);
extern u32 *ioapic_get_chip_base(int chip);
extern u32 ioapic_read(int chip, int address);
extern void ioapic_write(int chip, int address, u32 val);


/*
 * General APIC
 */
extern int apic_supported;
extern int apic_present;
extern ulong lapic_paddr;

extern void init_apic();


#endif
