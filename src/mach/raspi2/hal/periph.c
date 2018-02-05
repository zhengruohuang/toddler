#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "hal/include/bootparam.h"
#include "hal/include/fb.h"
#include "hal/include/print.h"
#include "hal/include/vector.h"
#include "hal/include/vecnum.h"


#define BCM2835_BASE            0x3f000000ul
#define BCM2836_LOCAL_BASE      0x40000000ul


/*
 * BCM2836 local control
 */
struct bcm2836_local_regs {
    // 0x00
    volatile u32 control;
    
    // 0x04
    volatile u32 reserved;
    
    // 0x08
    volatile u32 core_timer_prescaler;
    
    // 0x0c
    volatile u32 gpu_int_routing;
    
    // 0x10
    volatile u32 perf_mon_int_routing_set;
    volatile u32 perf_mon_int_routing_clear;
    
    // 0x18
    volatile u32 reserved2;
    
    // 0x1c
    volatile u32 core_timer_lo;
    volatile u32 core_timer_hi;
    
    // 0x24
    volatile u32 local_int_routing1;
    volatile u32 local_int_routing2;
    
    // 0x2c
    volatile u32 axi_outstanding_counters;
    volatile u32 axi_outstanding_irq;
    
    // 0x34
    volatile u32 local_timer_control;
    volatile u32 local_timer_write_flags;
    
    // 0x3c
    volatile u32 reserved3;
    
    // 0x40
    volatile u32 core0_timer_int_ctrl;
    volatile u32 core1_timer_int_ctrl;
    volatile u32 core2_timer_int_ctrl;
    volatile u32 core3_timer_int_ctrl;
    
    // 0x50
    volatile u32 core0_mailbox_int_ctrl;
    volatile u32 core1_mailbox_int_ctrl;
    volatile u32 core2_mailbox_int_ctrl;
    volatile u32 core3_mailbox_int_ctrl;
    
    // 0x60
    volatile u32 core0_irq_src;
    volatile u32 core1_irq_src;
    volatile u32 core2_irq_src;
    volatile u32 core3_irq_src;
    
    // 0x70
    volatile u32 core0_fiq_src;
    volatile u32 core1_fiq_src;
    volatile u32 core2_fiq_src;
    volatile u32 core3_fiq_src;
    
    // 0x80
    volatile u32 core0_mailbox_write_set[4];
    volatile u32 core1_mailbox_write_set[4];
    volatile u32 core2_mailbox_write_set[4];
    volatile u32 core3_mailbox_write_set[4];
    
    // 0xc0
    volatile u32 core0_mailbox_read_write_high_to_clear[4];
    volatile u32 core1_mailbox_read_write_high_to_clear[4];
    volatile u32 core2_mailbox_read_write_high_to_clear[4];
    volatile u32 core3_mailbox_read_write_high_to_clear[4];
} packedstruct;

static struct bcm2836_local_regs *bcm2836_local;

static void init_bcm2836_local()
{
    bcm2836_local = (void *)BCM2836_LOCAL_BASE;
    
//     // Init the four core timer ints - IRQ phys timer non secure
//     bcm2836_local->core0_timer_int_ctrl = 0x2;
//     bcm2836_local->core1_timer_int_ctrl = 0x2;
//     bcm2836_local->core2_timer_int_ctrl = 0x2;
//     bcm2836_local->core3_timer_int_ctrl = 0x2;
//     
//     // Init IRQ timer for 4 cores
//     bcm2836_local->core0_irq_src = 0x2;
//     bcm2836_local->core1_irq_src = 0x2;
//     bcm2836_local->core2_irq_src = 0x2;
//     bcm2836_local->core3_irq_src = 0x2;
    
    // Init the four core timer ints - FIQ phys timer non secure
    bcm2836_local->core0_timer_int_ctrl = 0x20;
    bcm2836_local->core1_timer_int_ctrl = 0x20;
    bcm2836_local->core2_timer_int_ctrl = 0x20;
    bcm2836_local->core3_timer_int_ctrl = 0x20;
    
    // Init FIQ timer for 4 cores
    bcm2836_local->core0_fiq_src = 0x2;
    bcm2836_local->core1_fiq_src = 0x2;
    bcm2836_local->core2_fiq_src = 0x2;
    bcm2836_local->core3_fiq_src = 0x2;
}


/*
 * ARM Timer: not implemented by QEMU
 */
#define BCM2835_ARM_TIMER_BASE  0xb400ul

struct bcm2835_arm_timer_ctrl {
    union {
        u32 value;
        struct {
            u32 reserved1       : 1;
            u32 wider_counter   : 1;    // 0: 16-bit counter, 1: 23-bit counter
            u32 pre_scale       : 2;    // 00: 1, 01: 16, 10: 256, 11: undefined
            u32 reserved2       : 1;
            u32 int_enabled     : 1;
            u32 reserved3       : 1;
            u32 timer_enabled   : 1;
            u32 reserved5       : 24;
        };
    };
};

struct bcm2835_arm_timer {
    volatile u32 load;
    volatile u32 value;
    volatile struct bcm2835_arm_timer_ctrl control;
    volatile u32 irq_clear;
    volatile u32 raw_irq;
    volatile u32 masked_irq;
    volatile u32 reload;
    volatile u32 pre_divider;
    volatile u32 free_running_counter;
};

static struct bcm2835_arm_timer *arm_timer;

static void init_arm_timer()
{
    arm_timer = (void *)(BCM2835_BASE + BCM2835_ARM_TIMER_BASE);
    
    // Disable timer
    arm_timer->control.value = 0;
}

static void start_arm_timer()
{
    // Construct the new ctrl reg
    struct bcm2835_arm_timer_ctrl ctrl;
    ctrl.value = 0;
    ctrl.wider_counter = 1;
    ctrl.timer_enabled = 1;
    ctrl.int_enabled = 1;
    ctrl.pre_scale = 1;
    
    // Load and enable
    arm_timer->load = 0x400;
    arm_timer->reload = 0x400;
    arm_timer->irq_clear = 1;
    arm_timer->control.value = ctrl.value;
    
    kprintf("Timer ctrl: %x\n", arm_timer->control.value);
}


/*
 * System timer: not implemented by QEMU
 */
#define BCM2835_SYS_TIMER_BASE  0x3000ul

struct bcm2835_sys_timer {
    volatile u32 control_status;
    volatile u32 counter_lo;
    volatile u32 counter_hi;
    volatile u32 compare0;
    volatile u32 compare1;
    volatile u32 compare2;
    volatile u32 compare3;
};

static struct bcm2835_sys_timer *sys_timer;

static void init_sys_timer()
{
    sys_timer = (void *)(BCM2835_BASE + BCM2835_SYS_TIMER_BASE);
}

static void start_sys_timer()
{
    sys_timer->compare0 = sys_timer->counter_lo + 1000000;
}


/*
 * Interrupt controller
 */
#define BCM2835_INT_CTRL_BASE               0xb200ul

#define BCM2835_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define BCM2835_BASIC_ARM_MAILBOX_IRQ       (1 << 1)
#define BCM2835_BASIC_ARM_DOORBELL_0_IRQ    (1 << 2)
#define BCM2835_BASIC_ARM_DOORBELL_1_IRQ    (1 << 3)
#define BCM2835_BASIC_GPU_0_HALTED_IRQ      (1 << 4)
#define BCM2835_BASIC_GPU_1_HALTED_IRQ      (1 << 5)
#define BCM2835_BASIC_ACCESS_ERROR_1_IRQ    (1 << 6)
#define BCM2835_BASIC_ACCESS_ERROR_0_IRQ    (1 << 7)

struct bcm2835_int_ctrl {
    volatile u32 irq_basic_pending;
    volatile u32 irq_pending1;
    volatile u32 irq_pending2;
    volatile u32 fiq_control;
    volatile u32 enable_irqs1;
    volatile u32 enable_irqs2;
    volatile u32 enable_basic_irqs;
    volatile u32 disable_irqs1;
    volatile u32 disable_irqs2;
    volatile u32 disable_basic_irqs;
};

static struct bcm2835_int_ctrl *int_ctrl;
static u32 int_ctrl_enabled_mask1, int_ctrl_enabled_mask2;
static int int_ctrl_wired_to_vector_map[64];

static int int_ctrl_get_pending_wired()
{
    int i, idx = 0;
    
    for (i = 0; i < 32; i++, idx++) {
        if (int_ctrl->irq_pending1 & (0x1 << i)) {
            return idx;
        }
    }
    
    for (i = 0; i < 32; i++, idx++) {
        if (int_ctrl->irq_pending2 & (0x1 << i)) {
            return idx;
        }
    }
    
    return -1;
}

static void int_ctrl_enable_wired(int wired)
{
    if (wired < 32) {
        int_ctrl_enabled_mask1 |= 0x1 << wired;
        int_ctrl->enable_irqs1 = int_ctrl_enabled_mask1;
    } else if (wired < 64) {
        int_ctrl_enabled_mask2 = 0x1 << (wired - 32);
        int_ctrl->enable_irqs2 |= int_ctrl_enabled_mask2;
    }
}

static void int_ctrl_disable_wired(int wired)
{
    if (wired < 32) {
        int_ctrl_enabled_mask1 &= ~(0x1 << wired);
        int_ctrl->disable_irqs1 = ~int_ctrl_enabled_mask1;
    } else if (wired < 64) {
        int_ctrl_enabled_mask2 &= ~(0x1 << (wired - 32));
        int_ctrl->disable_irqs2 = ~int_ctrl_enabled_mask2;
    }
}

static int int_ctrl_default_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    int wired = int_ctrl_get_pending_wired();
    kprintf("Interrupt not registered for wire %d, int to be disabled!\n", wired);
    
    if (wired < 64) {
        int_ctrl_disable_wired(wired);
    }
    
    return INT_HANDLE_TYPE_TAKEOVER;
}

static void init_int_ctrl()
{
    int_ctrl = (void *)(BCM2835_BASE + BCM2835_INT_CTRL_BASE);
    
    // Disable all interrupts
    int_ctrl_enabled_mask1 = int_ctrl_enabled_mask2 = 0;
    int_ctrl->disable_basic_irqs = 0xffffffff;
    int_ctrl->disable_irqs1 = 0xffffffff;
    int_ctrl->disable_irqs2 = 0xffffffff;
    
    // Init the wired to vector map with the default handler
    int i;
    for (i = 0; i < 64; i++) {
        int_ctrl_wired_to_vector_map[i] = INT_VECTOR_UNKNOWN_DEV;
    }
}

static void start_int_ctrl()
{
    // Enable the default handler
    set_int_vector(INT_VECTOR_UNKNOWN_DEV, int_ctrl_default_handler);
    
    // Enable necessary interrupts
    int_ctrl_enable_wired(57);
//     int_ctrl->enable_basic_irqs = 0xffffffff;
//     int_ctrl->enable_irqs1 = 0xDFFFFFFF;
//     int_ctrl->enable_irqs2 = 0xffffffff;
}

static int int_ctrl_register(int wired, int_handler handler)
{
    if (wired >= 64) {
        return -1;
    }
    
    int vector = alloc_int_vector(handler);
    int_ctrl_wired_to_vector_map[wired] = vector;
    
    return vector;
}

static int int_ctrl_get_pending_vector()
{
    int wired = int_ctrl_get_pending_wired();
    if (wired < 64) {
        return int_ctrl_wired_to_vector_map[wired];
    } else {
        return INT_VECTOR_UNKNOWN_DEV;
    }
}


/*
 * PL011 - UART
 */
#define BCM2835_PL011_BASE      (0x201000ul)

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
static int fb_enabled = 0;

static void bcm2835_pl011_write(char ch)
{
    // Wait until the UART has an empty space in the FIFO
    while (pl011->FR & 0x20);

    // Write the character to the FIFO for transmission
    pl011->DR = ch;
}

static int bcm2835_pl011_received()
{
    return !(pl011->FR & 0x10);
}

static u8 bcm2835_pl011_read()
{
    // Wait until the UART isn't busy
    while (pl011->FR & 0x8);

    // Return the data
    return (u8)pl011->DR;
}

static int bcm2835_pl011_int_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong count = 0;
    ulong data = 0;
    ulong byte = 0;
    
    // Disable all interrupts
    pl011->IMSC = 0;
    
    // Clear all interrupt status
    pl011->ICR = 0x7FF;
    
    while (bcm2835_pl011_received()) {
        byte = (ulong)bcm2835_pl011_read();
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
    
    kdi->dispatch_type = kdisp_interrupt;
    kdi->interrupt.irq = 1;
    kdi->interrupt.vector = context->vector;
    
    kdi->interrupt.param0 = (ulong)count;
    kdi->interrupt.param1 = data;
    kdi->interrupt.param2 = 0;
    
    // Enable receiving interrupt
    pl011->IMSC = 0x10;
    
    return INT_HANDLE_TYPE_KERNEL;
}

static void init_bcm2835_pl011()
{
    pl011 = (void *)(BCM2835_BASE + BCM2835_PL011_BASE);
    
    // Clear the receiving FIFO
    while (!(pl011->FR & 0x10)) {
        (void)pl011->DR;
    }
}

static void start_bcm2835_pl011()
{
    // Register the int handler
    int_ctrl_register(57, bcm2835_pl011_int_handler);
    
    // Enable receiving interrupt and disable irrevelent ones
    pl011->IMSC = 0x10;
    
    // Clear all interrupt status
    pl011->ICR = 0x7FF;
}


/*
 * Print
 */
void draw_char(char c)
{
    if (fb_enabled) {
        fb_draw_char(c);
    } else {
        bcm2835_pl011_write(c);
    }
}

static void init_print()
{
    struct boot_parameters *bp = get_bootparam();
    
    if (bp->video_mode == VIDEO_FRAMEBUFFER) {
        init_fb_draw_char((void *)bp->framebuffer_addr,
                          bp->res_x, bp->res_y, bp->bytes_per_pixel, bp->bytes_per_line);
        fb_enabled = 1;
    } else {
        init_bcm2835_pl011();
        fb_enabled = 0;
    }
}


/*
 * FIQ
 */
extern int is_generic_timer_asserted();

static int fiq_default_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    return INT_HANDLE_TYPE_TAKEOVER;
}

static void start_fiq()
{
    set_int_vector(INT_VECTOR_SPURIOUS_FIQ, fiq_default_handler);
}


/*
 * Top level
 */
void init_periph()
{
    init_print();
    
//     init_arm_timer();
//     init_sys_timer();
    init_bcm2836_local();
    init_int_ctrl();
}

void start_periph()
{
//     start_arm_timer();
//     start_sys_timer();
    start_bcm2835_pl011();
    start_fiq();
    start_int_ctrl();
}


/*
 * Interrupt
 */
int periph_get_irq_vector()
{
    int vector = int_ctrl_get_pending_vector();
    return vector;
}

weak_func int is_generic_timer_asserted()
{
    return 0;
}

int periph_get_fiq_vector()
{
    if (is_generic_timer_asserted()) {
        return INT_VECTOR_LOCAL_TIMER;
    } else {
        return INT_VECTOR_SPURIOUS_FIQ;
    }
}


/*
 * Num CPUs
 */
int periph_detect_num_cpus()
{
    return 4;
}

void periph_waekup_cpu(int cpu_id, ulong entry)
{
    *(volatile u32 *)(ulong)(0x4000008C + 0x10 * cpu_id) = (u32)entry;
}
