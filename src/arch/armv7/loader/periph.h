#ifndef __ARCH_ARMV7_LOADER_PERIPH_HH__
#define __ARCH_ARMV7_LOADER_PERIPH_HH__


/*
 * General
 */
#define PERIPHERAL_BASE     0x3F000000UL

#ifndef asmlinkage
#define asmlinkage __attribute__((regparm(0)))
#endif

#define NULL    ((void *)0)

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef volatile u32 rpi_reg_rw_t;
typedef volatile const u32 rpi_reg_ro_t;
typedef volatile u32 rpi_reg_wo_t;

typedef volatile u64 rpi_wreg_rw_t;
typedef volatile const u64 rpi_wreg_ro_t;


/*
 * Argument
 */
typedef void *  va_list;

#define va_start(ap, last)      ap = (va_list)(void *)(&last); ap += sizeof(last)
#define va_arg(ap, type)        *((type *)ap); ap += sizeof(type)
#define va_end(ap)
#define va_copy(dest, src)      dest = src


/*
 * GPIO
 */
#define GPIO_BASE       (PERIPHERAL_BASE + 0x200000UL)

#define LED_GPFSEL      GPFSEL4
#define LED_GPFBIT      21
#define LED_GPSET       GPSET1
#define LED_GPCLR       GPCLR1
#define LED_GPIO_BIT    15
    
typedef enum {
    FS_INPUT = 0,
    FS_OUTPUT,
    FS_ALT5,
    FS_ALT4,
    FS_ALT0,
    FS_ALT1,
    FS_ALT2,
    FS_ALT3,
} rpi_gpio_alt_function_t;

/* A mask to be able to clear the bits in the register before setting the
   value we require */
#define FS_MASK     (7)

typedef enum {
    RPI_GPIO0 = 0,
    RPI_GPIO1,
    RPI_GPIO2,
    RPI_GPIO3,
    RPI_GPIO4,
    RPI_GPIO5,
    RPI_GPIO6,
    RPI_GPIO7,
    RPI_GPIO8,
    RPI_GPIO9,
    RPI_GPIO10 = 10,
    RPI_GPIO11,
    RPI_GPIO12,
    RPI_GPIO13,
    RPI_GPIO14,
    RPI_GPIO15,
    RPI_GPIO16,
    RPI_GPIO17,
    RPI_GPIO18,
    RPI_GPIO19,
    RPI_GPIO20 = 20,
    RPI_GPIO21,
    RPI_GPIO22,
    RPI_GPIO23,
    RPI_GPIO24,
    RPI_GPIO25,
    RPI_GPIO26,
    RPI_GPIO27,
    RPI_GPIO28,
    RPI_GPIO29,
    RPI_GPIO30 = 30,
    RPI_GPIO31,
    RPI_GPIO32,
    RPI_GPIO33,
    RPI_GPIO34,
    RPI_GPIO35,
    RPI_GPIO36,
    RPI_GPIO37,
    RPI_GPIO38,
    RPI_GPIO39,
    RPI_GPIO40 = 40,
    RPI_GPIO41,
    RPI_GPIO42,
    RPI_GPIO43,
    RPI_GPIO44,
    RPI_GPIO45,
    RPI_GPIO46,
    RPI_GPIO47,
    RPI_GPIO48,
    RPI_GPIO49,
    RPI_GPIO50 = 50,
    RPI_GPIO51,
    RPI_GPIO52,
    RPI_GPIO53,
} rpi_gpio_pin_t;


/** The GPIO Peripheral is described in section 6 of the BCM2835 Peripherals
    documentation.
    There are 54 general-purpose I/O (GPIO) lines split into two banks. All
    GPIO pins have at least two alternative functions within BCM. The
    alternate functions are usually peripheral IO and a single peripheral
    may appear in each bank to allow flexibility on the choice of IO voltage.
    Details of alternative functions are given in section 6.2. Alternative
    Function Assignments.
    The GPIO peripheral has three dedicated interrupt lines. These lines are
    triggered by the setting of bits in the event detect status register. Each
    bank has its’ own interrupt line with the third line shared between all
    bits.
    The Alternate function table also has the pull state (pull-up/pull-down)
    which is applied after a power down. */
typedef struct {
    rpi_reg_rw_t    GPFSEL0;
    rpi_reg_rw_t    GPFSEL1;
    rpi_reg_rw_t    GPFSEL2;
    rpi_reg_rw_t    GPFSEL3;
    rpi_reg_rw_t    GPFSEL4;
    rpi_reg_rw_t    GPFSEL5;
    rpi_reg_ro_t    Reserved0;
    rpi_reg_wo_t    GPSET0;
    rpi_reg_wo_t    GPSET1;
    rpi_reg_ro_t    Reserved1;
    rpi_reg_wo_t    GPCLR0;
    rpi_reg_wo_t    GPCLR1;
    rpi_reg_ro_t    Reserved2;
    rpi_reg_wo_t    GPLEV0;
    rpi_reg_wo_t    GPLEV1;
    rpi_reg_ro_t    Reserved3;
    rpi_reg_wo_t    GPEDS0;
    rpi_reg_wo_t    GPEDS1;
    rpi_reg_ro_t    Reserved4;
    rpi_reg_wo_t    GPREN0;
    rpi_reg_wo_t    GPREN1;
    rpi_reg_ro_t    Reserved5;
    rpi_reg_wo_t    GPFEN0;
    rpi_reg_wo_t    GPFEN1;
    rpi_reg_ro_t    Reserved6;
    rpi_reg_wo_t    GPHEN0;
    rpi_reg_wo_t    GPHEN1;
    rpi_reg_ro_t    Reserved7;
    rpi_reg_wo_t    GPLEN0;
    rpi_reg_wo_t    GPLEN1;
    rpi_reg_ro_t    Reserved8;
    rpi_reg_wo_t    GPAREN0;
    rpi_reg_wo_t    GPAREN1;
    rpi_reg_ro_t    Reserved9;
    rpi_reg_wo_t    GPAFEN0;
    rpi_reg_wo_t    GPAFEN1;
    rpi_reg_ro_t    Reserved10;
    rpi_reg_wo_t    GPPUD;
    rpi_reg_wo_t    GPPUDCLK0;
    rpi_reg_wo_t    GPPUDCLK1;
    rpi_reg_ro_t    Reserved11;
} rpi_gpio_t;

typedef enum {
    RPI_IO_LO = 0,
    RPI_IO_HI,
    RPI_IO_ON,
    RPI_IO_OFF,
    RPI_IO_UNKNOWN,
} rpi_gpio_value_t;


/*
 * Timer
 */
#define RPI_SYSTIMER_BASE   (PERIPHERAL_BASE + 0x3000)

struct sys_timer {
    volatile u32 control_status;
    volatile u32 counter_lo;
    volatile u32 counter_hi;
    volatile u32 compare0;
    volatile u32 compare1;
    volatile u32 compare2;
    volatile u32 compare3;
};


/*
 * UART
 */
#define AUX_BASE    (PERIPHERAL_BASE + 0x215000)

/* Define the system clock frequency in MHz for the baud rate calculation.
   This is clearly defined on the BCM2835 datasheet errata page:
   http://elinux.org/BCM2835_datasheet_errata */
#define UART_FREQ    250000000

#define AUX_ENA_MINIUART            (1 << 0)
#define AUX_ENA_SPI1                (1 << 1)
#define AUX_ENA_SPI2                (1 << 2)

#define AUX_IRQ_SPI2                (1 << 2)
#define AUX_IRQ_SPI1                (1 << 1)
#define AUX_IRQ_MU                  (1 << 0)

#define AUX_MULCR_8BIT_MODE         (3 << 0)  /* See errata for this value */
#define AUX_MULCR_BREAK             (1 << 6)
#define AUX_MULCR_DLAB_ACCESS       (1 << 7)

#define AUX_MUMCR_RTS               (1 << 1)

#define AUX_MULSR_DATA_READY        (1 << 0)
#define AUX_MULSR_RX_OVERRUN        (1 << 1)
#define AUX_MULSR_TX_EMPTY          (1 << 5)
#define AUX_MULSR_TX_IDLE           (1 << 6)

#define AUX_MUMSR_CTS               (1 << 5)

#define AUX_MUCNTL_RX_ENABLE        (1 << 0)
#define AUX_MUCNTL_TX_ENABLE        (1 << 1)
#define AUX_MUCNTL_RTS_FLOW         (1 << 2)
#define AUX_MUCNTL_CTS_FLOW         (1 << 3)
#define AUX_MUCNTL_RTS_FIFO         (3 << 4)
#define AUX_MUCNTL_RTS_ASSERT       (1 << 6)
#define AUX_MUCNTL_CTS_ASSERT       (1 << 7)

#define AUX_MUSTAT_SYMBOL_AV        (1 << 0)
#define AUX_MUSTAT_SPACE_AV         (1 << 1)
#define AUX_MUSTAT_RX_IDLE          (1 << 2)
#define AUX_MUSTAT_TX_IDLE          (1 << 3)
#define AUX_MUSTAT_RX_OVERRUN       (1 << 4)
#define AUX_MUSTAT_TX_FIFO_FULL     (1 << 5)
#define AUX_MUSTAT_RTS              (1 << 6)
#define AUX_MUSTAT_CTS              (1 << 7)
#define AUX_MUSTAT_TX_EMPTY         (1 << 8)
#define AUX_MUSTAT_TX_DONE          (1 << 9)
#define AUX_MUSTAT_RX_FIFO_LEVEL    (7 << 16)
#define AUX_MUSTAT_TX_FIFO_LEVEL    (7 << 24)

#define FSEL0(x)        (x)
#define FSEL1(x)        (x << 3)
#define FSEL2(x)        (x << 6)
#define FSEL3(x)        (x << 9)
#define FSEL4(x)        (x << 12)
#define FSEL5(x)        (x << 15)
#define FSEL6(x)        (x << 18)
#define FSEL7(x)        (x << 21)
#define FSEL8(x)        (x << 24)
#define FSEL9(x)        (x << 27)

#define FSEL10(x)       (x)
#define FSEL11(x)       (x << 3)
#define FSEL12(x)       (x << 6)
#define FSEL13(x)       (x << 9)
#define FSEL14(x)       (x << 12)
#define FSEL15(x)       (x << 15)
#define FSEL16(x)       (x << 18)
#define FSEL17(x)       (x << 21)
#define FSEL18(x)       (x << 24)
#define FSEL19(x)       (x << 27)

#define FSEL20(x)       (x)
#define FSEL21(x)       (x << 3)
#define FSEL22(x)       (x << 6)
#define FSEL23(x)       (x << 9)
#define FSEL24(x)       (x << 12)
#define FSEL25(x)       (x << 15)
#define FSEL26(x)       (x << 18)
#define FSEL27(x)       (x << 21)
#define FSEL28(x)       (x << 24)
#define FSEL29(x)       (x << 27)

#define FSEL30(x)       (x)
#define FSEL31(x)       (x << 3)
#define FSEL32(x)       (x << 6)
#define FSEL33(x)       (x << 9)
#define FSEL34(x)       (x << 12)
#define FSEL35(x)       (x << 15)
#define FSEL36(x)       (x << 18)
#define FSEL37(x)       (x << 21)
#define FSEL38(x)       (x << 24)
#define FSEL39(x)       (x << 27)

#define FSEL40(x)       (x)
#define FSEL41(x)       (x << 3)
#define FSEL42(x)       (x << 6)
#define FSEL43(x)       (x << 9)
#define FSEL44(x)       (x << 12)
#define FSEL45(x)       (x << 15)
#define FSEL46(x)       (x << 18)
#define FSEL47(x)       (x << 21)
#define FSEL48(x)       (x << 24)
#define FSEL49(x)       (x << 27)

#define FSEL50(x)       (x)
#define FSEL51(x)       (x << 3)
#define FSEL52(x)       (x << 6)
#define FSEL53(x)       (x << 9)

typedef struct {
    volatile unsigned int IRQ;
    volatile unsigned int ENABLES;

    volatile unsigned int reserved1[((0x40 - 0x04) / 4) - 1];

    volatile unsigned int MU_IO;
    volatile unsigned int MU_IER;
    volatile unsigned int MU_IIR;
    volatile unsigned int MU_LCR;
    volatile unsigned int MU_MCR;
    volatile unsigned int MU_LSR;
    volatile unsigned int MU_MSR;
    volatile unsigned int MU_SCRATCH;
    volatile unsigned int MU_CNTL;
    volatile unsigned int MU_STAT;
    volatile unsigned int MU_BAUD;

    volatile unsigned int reserved2[(0x80 - 0x68) / 4];

    volatile unsigned int SPI0_CNTL0;
    volatile unsigned int SPI0_CNTL1;
    volatile unsigned int SPI0_STAT;
    volatile unsigned int SPI0_IO;
    volatile unsigned int SPI0_PEEK;

    volatile unsigned int reserved3[(0xC0 - 0x94) / 4];

    volatile unsigned int SPI1_CNTL0;
    volatile unsigned int SPI1_CNTL1;
    volatile unsigned int SPI1_STAT;
    volatile unsigned int SPI1_IO;
    volatile unsigned int SPI1_PEEK;
} aux_t;


/*
 * Mailbox
 */
#define RPI_MAILBOX0_BASE    (PERIPHERAL_BASE + 0xB880)

/* The available mailbox channels in the BCM2835 Mailbox interface.
   See https://github.com/raspberrypi/firmware/wiki/Mailboxes for
   information */
typedef enum {
    MB0_POWER_MANAGEMENT = 0,
    MB0_FRAMEBUFFER,
    MB0_VIRTUAL_UART,
    MB0_VCHIQ,
    MB0_LEDS,
    MB0_BUTTONS,
    MB0_TOUCHSCREEN,
    MB0_UNUSED,
    MB0_TAGS_ARM_TO_VC,
    MB0_TAGS_VC_TO_ARM,
} mailbox0_channel_t;

/* These defines come from the Broadcom Videocode driver source code, see:
   brcm_usrlib/dag/vmcsx/vcinclude/bcm2708_chip/arm_control.h */
enum mailbox_status_reg_bits {
    ARM_MS_FULL  = 0x80000000,
    ARM_MS_EMPTY = 0x40000000,
    ARM_MS_LEVEL = 0x400000FF,
};

/* Define a structure which defines the register access to a mailbox.
   Not all mailboxes support the full register set! */
typedef struct {
    volatile unsigned int Read;
    volatile unsigned int reserved1[((0x90 - 0x80) / 4) - 1];
    volatile unsigned int Poll;
    volatile unsigned int Sender;
    volatile unsigned int Status;
    volatile unsigned int Configuration;
    volatile unsigned int Write;
} mailbox_t;


/*
 * Tag
 */
typedef enum {
    /* Videocore */
    TAG_GET_FIRMWARE_VERSION = 0x1,

    /* Hardware */
    TAG_GET_BOARD_MODEL = 0x10001,
    TAG_GET_BOARD_REVISION,
    TAG_GET_BOARD_MAC_ADDRESS,
    TAG_GET_BOARD_SERIAL,
    TAG_GET_ARM_MEMORY,
    TAG_GET_VC_MEMORY,
    TAG_GET_CLOCKS,

    /* Config */
    TAG_GET_COMMAND_LINE = 0x50001,

    /* Shared resource management */
    TAG_GET_DMA_CHANNELS = 0x60001,

    /* Power */
    TAG_GET_POWER_STATE = 0x20001,
    TAG_GET_TIMING,
    TAG_SET_POWER_STATE = 0x28001,

    /* Clocks */
    TAG_GET_CLOCK_STATE = 0x30001,
    TAG_SET_CLOCK_STATE = 0x38001,
    TAG_GET_CLOCK_RATE = 0x30002,
    TAG_SET_CLOCK_RATE = 0x38002,
    TAG_GET_MAX_CLOCK_RATE = 0x30004,
    TAG_GET_MIN_CLOCK_RATE = 0x30007,
    TAG_GET_TURBO = 0x30009,
    TAG_SET_TURBO = 0x38009,

    /* Voltage */
    TAG_GET_VOLTAGE = 0x30003,
    TAG_SET_VOLTAGE = 0x38003,
    TAG_GET_MAX_VOLTAGE = 0x30005,
    TAG_GET_MIN_VOLTAGE = 0x30008,
    TAG_GET_TEMPERATURE = 0x30006,
    TAG_GET_MAX_TEMPERATURE = 0x3000A,
    TAG_ALLOCATE_MEMORY = 0x3000C,
    TAG_LOCK_MEMORY = 0x3000D,
    TAG_UNLOCK_MEMORY = 0x3000E,
    TAG_RELEASE_MEMORY = 0x3000F,
    TAG_EXECUTE_CODE = 0x30010,
    TAG_GET_DISPMANX_MEM_HANDLE = 0x30014,
    TAG_GET_EDID_BLOCK = 0x30020,

    /* Framebuffer */
    TAG_ALLOCATE_BUFFER = 0x40001,
    TAG_RELEASE_BUFFER = 0x48001,
    TAG_BLANK_SCREEN = 0x40002,
    TAG_GET_PHYSICAL_SIZE = 0x40003,
    TAG_TEST_PHYSICAL_SIZE = 0x44003,
    TAG_SET_PHYSICAL_SIZE = 0x48003,
    TAG_GET_VIRTUAL_SIZE = 0x40004,
    TAG_TEST_VIRTUAL_SIZE = 0x44004,
    TAG_SET_VIRTUAL_SIZE = 0x48004,
    TAG_GET_DEPTH = 0x40005,
    TAG_TEST_DEPTH = 0x44005,
    TAG_SET_DEPTH = 0x48005,
    TAG_GET_PIXEL_ORDER = 0x40006,
    TAG_TEST_PIXEL_ORDER = 0x44006,
    TAG_SET_PIXEL_ORDER = 0x48006,
    TAG_GET_ALPHA_MODE = 0x40007,
    TAG_TEST_ALPHA_MODE = 0x44007,
    TAG_SET_ALPHA_MODE = 0x48007,
    TAG_GET_PITCH = 0x40008,
    TAG_GET_VIRTUAL_OFFSET = 0x40009,
    TAG_TEST_VIRTUAL_OFFSET = 0x44009,
    TAG_SET_VIRTUAL_OFFSET = 0x48009,
    TAG_GET_OVERSCAN = 0x4000A,
    TAG_TEST_OVERSCAN = 0x4400A,
    TAG_SET_OVERSCAN = 0x4800A,
    TAG_GET_PALETTE = 0x4000B,
    TAG_TEST_PALETTE = 0x4400B,
    TAG_SET_PALETTE = 0x4800B,
    TAG_SET_CURSOR_INFO = 0x8011,
    TAG_SET_CURSOR_STATE = 0x8010
} rpi_mailbox_tag_t;


typedef enum {
    TAG_STATE_REQUEST = 0,
    TAG_STATE_RESPONSE = 1,
} rpi_tag_state_t;


typedef enum {
    PT_OSIZE = 0,
    PT_OREQUEST_OR_RESPONSE = 1,
} rpi_tag_buffer_offset_t;

typedef enum {
    T_OIDENT = 0,
    T_OVALUE_SIZE = 1,
    T_ORESPONSE = 2,
    T_OVALUE = 3,
} rpi_tag_offset_t;

typedef struct {
    u32 tag;
    u32 byte_length;
    union {
        u32 value_32;
        unsigned char buffer_8[256];
        u32 buffer_32[64];
    } data;
} rpi_mailbox_property_t;

typedef enum {
    TAG_CLOCK_RESERVED = 0,
    TAG_CLOCK_EMMC,
    TAG_CLOCK_UART,
    TAG_CLOCK_ARM,
    TAG_CLOCK_CORE,
    TAG_CLOCK_V3D,
    TAG_CLOCK_H264,
    TAG_CLOCK_ISP,
    TAG_CLOCK_SDRAM,
    TAG_CLOCK_PIXEL,
    TAG_CLOCK_PWM,
} rpi_tag_clock_id_t;


/*
 * Functions
 */
extern void gpio_set_output(rpi_gpio_pin_t pin);
extern void gpio_set_input(rpi_gpio_pin_t pin);
extern rpi_gpio_value_t gpio_get(rpi_gpio_pin_t pin);
extern void gpio_set(rpi_gpio_pin_t pin, rpi_gpio_value_t value);
extern void gpio_toggle(rpi_gpio_pin_t pin);

extern void led_on();
extern void led_off();
extern void led_blink(int times, int interval_ms);

extern void uart_write(char ch);

extern void mailbox0_write(mailbox0_channel_t channel, u32 value);
extern u32 mailbox0_read(mailbox0_channel_t channel);

extern void property_init();
extern asmlinkage void property_tag(rpi_mailbox_tag_t tag, ...);
extern u32 property_process();
extern rpi_mailbox_property_t *property_get(rpi_mailbox_tag_t tag);

extern asmlinkage void lprintf(char *fmt, ...);

extern void init_periph();


#endif
