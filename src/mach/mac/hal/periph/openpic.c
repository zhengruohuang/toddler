#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/debug.h"
#include "hal/include/vector.h"
#include "hal/include/vecnum.h"
#include "hal/include/periph.h"


/*
 * References
 * NetBSD
 */
/* feature reporting reg 0 */
#define OPENPIC_FEATURE         0x1000

/* global config reg 0 */
#define OPENPIC_CONFIG          0x1020
#define OPENPIC_CONFIG_RESET    0x80000000
#define OPENPIC_CONFIG_8259_PASSTHRU_DISABLE    0x20000000

/* interrupt configuration mode (direct or serial) */
#define OPENPIC_ICR                     0x1030
#define OPENPIC_ICR_SERIAL_MODE         (1 << 27)
#define OPENPIC_ICR_SERIAL_RATIO_MASK   (0x7 << 28)
#define OPENPIC_ICR_SERIAL_RATIO_SHIFT  28

/* vendor ID */
#define OPENPIC_VENDOR_ID           0x1080

/* processor initialization reg */
#define OPENPIC_PROC_INIT           0x1090

/* IPI vector/priority reg */
#define OPENPIC_IPI_VECTOR(ipi)     (0x10a0 + (ipi) * 0x10)

/* spurious intr. vector */
#define OPENPIC_SPURIOUS_VECTOR     0x10e0

/* interrupt vector/priority reg */
#define OPENPIC_SRC_VECTOR(irq)     (0x10000 + (irq) * 0x20)

#define OPENPIC_SENSE_LEVEL         0x00400000
#define OPENPIC_SENSE_EDGE          0x00000000
#define OPENPIC_POLARITY_POSITIVE   0x00800000
#define OPENPIC_POLARITY_NEGATIVE   0x00000000
#define OPENPIC_IMASK               0x80000000
#define OPENPIC_ACTIVITY            0x40000000
#define OPENPIC_PRIORITY_MASK       0x000f0000
#define OPENPIC_PRIORITY_SHIFT      16
#define OPENPIC_VECTOR_MASK         0x000000ff

/* interrupt destination cpu */
#define OPENPIC_IDEST(irq)          (0x10010 + (irq) * 0x20)

/* IPI command reg */
#define OPENPIC_IPI(cpu, ipi)       (0x20040 + (cpu) * 0x1000 + (ipi))

/* current task priority reg */
#define OPENPIC_CPU_PRIORITY(cpu)   (0x20080 + (cpu) * 0x1000)
#define OPENPIC_CPU_PRIORITY_MASK   0x0000000f

/* interrupt acknowledge reg */
#define OPENPIC_IACK(cpu)           (0x200a0 + (cpu) * 0x1000)

/* end of interrupt reg */
#define OPENPIC_EOI(cpu)            (0x200b0 + (cpu) * 0x1000)

#define IPI_VECTOR 128



static volatile u32 *pic = NULL;

static int default_pic_vector = 0;
static int wired_to_vector_map[64];


static u32 swap_endian32(u32 val)
{
    u32 rr = val & 0xff;
    u32 rl = (val >> 8) & 0xff;
    u32 lr = (val >> 16) & 0xff;
    u32 ll = (val >> 24) & 0xff;
    
    u32 swap = (rr << 24) | (rl << 16) | (lr << 8) | ll;
    
    return swap;
}

static u32 openpic_read(ulong reg)
{
    volatile u32 *addr = (void *)((ulong)pic + reg);
    u32 val = swap_endian32(*addr);
    __asm__ __volatile__ ( "eieio;" );
    return val;
}

static void openpic_write(ulong reg, u32 val)
{
    volatile u32 *addr = (void *)((ulong)pic + reg);
    *addr = swap_endian32(val);
    __asm__ __volatile__ ( "eieio;" );
}

static void openpic_set_priority(int cpu, int pri)
{
    u32 x;
    x = openpic_read(OPENPIC_CPU_PRIORITY(cpu));
    x &= ~OPENPIC_CPU_PRIORITY_MASK;
    x |= pri;
    openpic_write(OPENPIC_CPU_PRIORITY(cpu), x);
}

static int openpic_read_irq(int cpu)
{
    return openpic_read(OPENPIC_IACK(cpu)) & OPENPIC_VECTOR_MASK;
}

static void openpic_ack(int cpu)
{
    openpic_write(OPENPIC_EOI(cpu), 0);
    openpic_read(OPENPIC_EOI(cpu));
}

static void enable_int(int num)
{
    u32 x;
    if (num == IPI_VECTOR) return;

    x = openpic_read(OPENPIC_SRC_VECTOR(num));
    x &= ~OPENPIC_IMASK;
    openpic_write(OPENPIC_SRC_VECTOR(num), x);
}

static void disable_int(int num)
{
    u32 x;
    if (num == IPI_VECTOR) return;

    x = openpic_read(OPENPIC_SRC_VECTOR(num));
    x |= OPENPIC_IMASK;
    openpic_write(OPENPIC_SRC_VECTOR(num), x);
}

static int get_pending()
{
    int pending = openpic_read_irq(0);
    return pending;
}

static int default_int_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    int wired = get_pending();
    panic("Unregistered pending IRQ: %d\n", wired);
    
    return INT_HANDLE_TYPE_KERNEL;
}

void openpic_eoi(int wired)
{
    openpic_ack(0);
}

int openpic_register_wired(int wired, int_handler handler)
{
    if (wired >= 64) {
        return -1;
    }
    
    int vector = alloc_int_vector(handler);
    wired_to_vector_map[wired] = vector;
    
    return vector;
}

int openpic_get_vector()
{
    int wired = get_pending();
    
    if (wired_to_vector_map[wired]) {
        return wired_to_vector_map[wired];
    } else {
        return default_pic_vector;
    }
}

void start_openpic()
{
    // Register the default handler
    default_pic_vector = alloc_int_vector(default_int_handler);
    
    // Enable all interrupts
    int num_ints = IPI_VECTOR + 1;
    int irq;
    for (irq = 0; irq < num_ints - 1; irq++) {
        u32 x = openpic_read(OPENPIC_SRC_VECTOR(irq));
        x &= ~OPENPIC_IMASK;
        openpic_write(OPENPIC_SRC_VECTOR(irq), x);
    }
    
    for (ulong offset = 4; offset < 0x40000; offset += 0x4ul) {
        u32 x = openpic_read(offset);
        if (x && x != 0xfffffffful) kprintf("Offset @ %lx: %x\n", offset, x);
        //kprintf("%x", x);
    }
//     while (1);
}

void init_openpic()
{
    struct boot_parameters *bp = get_bootparam();
    pic = (void *)bp->int_ctrl_addr;
//     pic = (void *)0x80040000ul;
    
    // Clear the wired to vector table
    int i;
    for (i = 0; i < 64; i++) {
        wired_to_vector_map[i] = 0;
    }
    
    for (ulong offset = 4; offset < 0x40000; offset += 0x4ul) {
        u32 x = openpic_read(offset);
        if (x && x != 0xfffffffful) kprintf("Offset @ %lx: %x\n", offset, x);
        //kprintf("%x", x);
    }
//     while (1);
    
    // Report OpenPIC features
    u32 x = openpic_read(OPENPIC_FEATURE);
    if (((x & 0x07ff0000) >> 16) == 0) {
        panic("init_openpic() called on distributed openpic");
    }

    kprintf("OpenPIC Version 1.%d: "
        "Supports %d CPUs and %d interrupt sources.\n",
        x & 0xff, ((x & 0x1f00) >> 8) + 1, ((x & 0x07ff0000) >> 16) + 1);
    
//     while (1);
    
    // Init the open PIC
//     openpic_set_priority(0, 15);
    
    int num_ints = IPI_VECTOR + 1;
    int irq;
    for (irq = 0; irq < num_ints - 1; irq++) {
        // make sure to keep disabled
        openpic_write(OPENPIC_SRC_VECTOR(irq), OPENPIC_IMASK);
        
        // send all interrupts to CPU 0
        openpic_write(OPENPIC_IDEST(irq), 1 << 0);
    }

//     int passthrough = 0;
//     u32 x = openpic_read(OPENPIC_CONFIG);
//     if (passthrough) {
//         x &= ~OPENPIC_CONFIG_8259_PASSTHRU_DISABLE;
//     } else {
//         x |= OPENPIC_CONFIG_8259_PASSTHRU_DISABLE;
//     }
//     openpic_write(OPENPIC_CONFIG, x);

//     openpic_write(OPENPIC_SPURIOUS_VECTOR, 0xff);

    openpic_set_priority(0, 0);

    // clear all pending interrunts
    for (irq = 0; irq < num_ints; irq++) {
        openpic_read_irq(0);
        openpic_eoi(0);
    }
    
    // Establish all IRQs
    for (irq = 0; irq < num_ints - 1; irq++) {
        u32 x = irq;
        x |= OPENPIC_IMASK;
        x |= 1 << OPENPIC_PRIORITY_SHIFT;
        openpic_write(OPENPIC_SRC_VECTOR(irq), x);
    }
}
