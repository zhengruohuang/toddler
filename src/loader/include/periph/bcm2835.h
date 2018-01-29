#ifndef __LOADER_BCM2835_H__
#define __LOADER_BCM2835_H__


#include "common/include/data.h"


/*
 * Top level
 */
extern void init_bcm2835(ulong bcm2835_base, ulong bcm2835_end);


/*
 * Timer
 */
void init_bcm2835_timer(ulong bcm2835_base);
extern void bcm2835_delay(u32 d);
extern u64 bcm2835_timer_read();


/*
 * GPIO
 */
enum bcm2835_gpio_func {
    BCM2835_FUNC_INPUT = 0,
    BCM2835_FUNC_OUTPUT,
    BCM2835_FUNC_ALT5,
    BCM2835_FUNC_ALT4,
    BCM2835_FUNC_ALT0,
    BCM2835_FUNC_ALT1,
    BCM2835_FUNC_ALT2,
    BCM2835_FUNC_ALT3,
};

enum bcm2835_gpio_pin {
    BCM2835_GPIO_PIN0 = 0,
    BCM2835_GPIO_PIN1,
    BCM2835_GPIO_PIN2,
    BCM2835_GPIO_PIN3,
    BCM2835_GPIO_PIN4,
    BCM2835_GPIO_PIN5,
    BCM2835_GPIO_PIN6,
    BCM2835_GPIO_PIN7,
    BCM2835_GPIO_PIN8,
    BCM2835_GPIO_PIN9,
    BCM2835_GPIO_PIN10 = 10,
    BCM2835_GPIO_PIN11,
    BCM2835_GPIO_PIN12,
    BCM2835_GPIO_PIN13,
    BCM2835_GPIO_PIN14,
    BCM2835_GPIO_PIN15,
    BCM2835_GPIO_PIN16,
    BCM2835_GPIO_PIN17,
    BCM2835_GPIO_PIN18,
    BCM2835_GPIO_PIN19,
    BCM2835_GPIO_PIN20 = 20,
    BCM2835_GPIO_PIN21,
    BCM2835_GPIO_PIN22,
    BCM2835_GPIO_PIN23,
    BCM2835_GPIO_PIN24,
    BCM2835_GPIO_PIN25,
    BCM2835_GPIO_PIN26,
    BCM2835_GPIO_PIN27,
    BCM2835_GPIO_PIN28,
    BCM2835_GPIO_PIN29,
    BCM2835_GPIO_PIN30 = 30,
    BCM2835_GPIO_PIN31,
    BCM2835_GPIO_PIN32,
    BCM2835_GPIO_PIN33,
    BCM2835_GPIO_PIN34,
    BCM2835_GPIO_PIN35,
    BCM2835_GPIO_PIN36,
    BCM2835_GPIO_PIN37,
    BCM2835_GPIO_PIN38,
    BCM2835_GPIO_PIN39,
    BCM2835_GPIO_PIN40 = 40,
    BCM2835_GPIO_PIN41,
    BCM2835_GPIO_PIN42,
    BCM2835_GPIO_PIN43,
    BCM2835_GPIO_PIN44,
    BCM2835_GPIO_PIN45,
    BCM2835_GPIO_PIN46,
    BCM2835_GPIO_PIN47,
    BCM2835_GPIO_PIN48,
    BCM2835_GPIO_PIN49,
    BCM2835_GPIO_PIN50 = 50,
    BCM2835_GPIO_PIN51,
    BCM2835_GPIO_PIN52,
    BCM2835_GPIO_PIN53,
};

enum bcm2835_gpio_value {
    BCM2835_GPIO_VAL_LO = 0,
    BCM2835_GPIO_VAL_HI,
    BCM2835_GPIO_VAL_ON,
    BCM2835_GPIO_VAL_OFF,
    BCM2835_GPIO_VAL_UNKNOWN,
};

extern void init_bcm2835_gpio(ulong bcm2835_base);

extern void bcm2835_gpio_enable_uart();
extern void bcm2835_gpio_enable_pl011();

extern void bcm2835_gpio_led_set();
extern void bcm2835_gpio_led_clear();


/*
 * AUX
 */
#define BCM2835_AUX_ENABLE_UART1    0x01
#define BCM2835_AUX_ENABLE_SPI0     0x02    // SPI0 (SPI1 in the device)
#define BCM2835_AUX_ENABLE_SPI1     0x04    // SPI1 (SPI2 in the device)


/*
 * UART
 */
extern void init_bcm2835_uart(ulong bcm2835_base);
extern u8 bcm2835_uart_read();
extern void bcm2835_uart_write(u8 ch);


/*
 * PL011
 */
extern void init_bcm2835_pl011(ulong bcm2835_base);
extern u8 bcm2835_pl011_read();
extern void bcm2835_pl011_write(u8 ch);


/*
 * LED
 */
extern void init_bcm2835_led();
extern void bcm2835_led_on();
extern void bcm2835_led_off();
extern void bcm2835_led_blink(int times, int interval_ms);


/*
 * Mailbox
 */
#define BCM2835_MAILBOX0_BASE   (0xb880)

// The available mailbox channels
// See https://github.com/raspberrypi/firmware/wiki/Mailboxes for information
enum bcm2835_mailbox0_channel {
    BCM2835_MAILBOX0_POWER_MANAGEMENT = 0,
    BCM2835_MAILBOX0_FRAMEBUFFER,
    BCM2835_MAILBOX0_VIRTUAL_UART,
    BCM2835_MAILBOX0_VCHIQ,
    BCM2835_MAILBOX0_LEDS,
    BCM2835_MAILBOX0_BUTTONS,
    BCM2835_MAILBOX0_TOUCHSCREEN,
    BCM2835_MAILBOX0_UNUSED,
    BCM2835_MAILBOX0_TAGS_ARM_TO_VC,
    BCM2835_MAILBOX0_TAGS_VC_TO_ARM,
};

// These defines come from the Broadcom Videocode driver source code, see:
// brcm_usrlib/dag/vmcsx/vcinclude/bcm2708_chip/arm_control.h
enum bcm2835_mailbox0_status {
    BCM2835_MAILBOX_FULL  = 0x80000000,
    BCM2835_MAILBOX_EMPTY = 0x40000000,
    BCM2835_MAILBOX_LEVEL = 0x400000FF,
};

struct bcm2835_mailbox {
    volatile unsigned int read;
    volatile unsigned int reserved1[((0x90 - 0x80) / 4) - 1];
    volatile unsigned int poll;
    volatile unsigned int sender;
    volatile unsigned int status;
    volatile unsigned int config;
    volatile unsigned int write;
};

enum bcm2835_mailbox_tag {
    // Videocore
    BCM2835_TAG_GET_FIRMWARE_VERSION = 0x1,

    // Hardware
    BCM2835_TAG_GET_BOARD_MODEL = 0x10001,
    BCM2835_TAG_GET_BOARD_REVISION,
    BCM2835_TAG_GET_BOARD_MAC_ADDRESS,
    BCM2835_TAG_GET_BOARD_SERIAL,
    BCM2835_TAG_GET_ARM_MEMORY,
    BCM2835_TAG_GET_VC_MEMORY,
    BCM2835_TAG_GET_CLOCKS,

    // Config
    BCM2835_TAG_GET_COMMAND_LINE = 0x50001,

    // Shared resource management
    BCM2835_TAG_GET_DMA_CHANNELS = 0x60001,

    // Power
    BCM2835_TAG_GET_POWER_STATE = 0x20001,
    BCM2835_TAG_GET_TIMING,
    BCM2835_TAG_SET_POWER_STATE = 0x28001,

    // Clocks
    BCM2835_TAG_GET_CLOCK_STATE = 0x30001,
    BCM2835_TAG_SET_CLOCK_STATE = 0x38001,
    BCM2835_TAG_GET_CLOCK_RATE = 0x30002,
    BCM2835_TAG_SET_CLOCK_RATE = 0x38002,
    BCM2835_TAG_GET_MAX_CLOCK_RATE = 0x30004,
    BCM2835_TAG_GET_MIN_CLOCK_RATE = 0x30007,
    BCM2835_TAG_GET_TURBO = 0x30009,
    BCM2835_TAG_SET_TURBO = 0x38009,

    // Voltage
    BCM2835_TAG_GET_VOLTAGE = 0x30003,
    BCM2835_TAG_SET_VOLTAGE = 0x38003,
    BCM2835_TAG_GET_MAX_VOLTAGE = 0x30005,
    BCM2835_TAG_GET_MIN_VOLTAGE = 0x30008,
    BCM2835_TAG_GET_TEMPERATURE = 0x30006,
    BCM2835_TAG_GET_MAX_TEMPERATURE = 0x3000A,
    BCM2835_TAG_ALLOCATE_MEMORY = 0x3000C,
    BCM2835_TAG_LOCK_MEMORY = 0x3000D,
    BCM2835_TAG_UNLOCK_MEMORY = 0x3000E,
    BCM2835_TAG_RELEASE_MEMORY = 0x3000F,
    BCM2835_TAG_EXECUTE_CODE = 0x30010,
    BCM2835_TAG_GET_DISPMANX_MEM_HANDLE = 0x30014,
    BCM2835_TAG_GET_EDID_BLOCK = 0x30020,

    // Framebuffer
    BCM2835_TAG_ALLOCATE_BUFFER = 0x40001,
    BCM2835_TAG_RELEASE_BUFFER = 0x48001,
    BCM2835_TAG_BLANK_SCREEN = 0x40002,
    BCM2835_TAG_GET_PHYSICAL_SIZE = 0x40003,
    BCM2835_TAG_TEST_PHYSICAL_SIZE = 0x44003,
    BCM2835_TAG_SET_PHYSICAL_SIZE = 0x48003,
    BCM2835_TAG_GET_VIRTUAL_SIZE = 0x40004,
    BCM2835_TAG_TEST_VIRTUAL_SIZE = 0x44004,
    BCM2835_TAG_SET_VIRTUAL_SIZE = 0x48004,
    BCM2835_TAG_GET_DEPTH = 0x40005,
    BCM2835_TAG_TEST_DEPTH = 0x44005,
    BCM2835_TAG_SET_DEPTH = 0x48005,
    BCM2835_TAG_GET_PIXEL_ORDER = 0x40006,
    BCM2835_TAG_TEST_PIXEL_ORDER = 0x44006,
    BCM2835_TAG_SET_PIXEL_ORDER = 0x48006,
    BCM2835_TAG_GET_ALPHA_MODE = 0x40007,
    BCM2835_TAG_TEST_ALPHA_MODE = 0x44007,
    BCM2835_TAG_SET_ALPHA_MODE = 0x48007,
    BCM2835_TAG_GET_PITCH = 0x40008,
    BCM2835_TAG_GET_VIRTUAL_OFFSET = 0x40009,
    BCM2835_TAG_TEST_VIRTUAL_OFFSET = 0x44009,
    BCM2835_TAG_SET_VIRTUAL_OFFSET = 0x48009,
    BCM2835_TAG_GET_OVERSCAN = 0x4000A,
    BCM2835_TAG_TEST_OVERSCAN = 0x4400A,
    BCM2835_TAG_SET_OVERSCAN = 0x4800A,
    BCM2835_TAG_GET_PALETTE = 0x4000B,
    BCM2835_TAG_TEST_PALETTE = 0x4400B,
    BCM2835_TAG_SET_PALETTE = 0x4800B,
    BCM2835_TAG_SET_CURSOR_INFO = 0x8011,
    BCM2835_TAG_SET_CURSOR_STATE = 0x8010
};

enum bcm2835_tag_state {
    BCM2835_TAG_STATE_REQUEST = 0,
    BCM2835_TAG_STATE_RESPONSE,
};


enum bcm2835_tag_buffer_offset {
    BCM2835_TAG_BUF_OFFSET_SIZE_ = 0,
    BCM2835_TAG_BUF_OFFSET_REQUEST_OR_RESPONSE,
};

enum bcm2835_tag_offset {
    BCM2835_TAG_OFFSET_IDENT = 0,
    BCM2835_TAG_OFFSET_VALUE_SIZE,
    BCM2835_TAG_OFFSET_RESPONSE,
    BCM2835_TAG_OFFSET_VALUE,
};

struct bcm2835_mailbox_property {
    u32 tag;
    u32 byte_length;
    union {
        u32 val32;
        unsigned char buf8[256];
        u32 buf32[64];
    } data;
};

enum bcm2835_clock_type {
    BCM2835_CLOCK_RESERVED = 0,
    BCM2835_CLOCK_EMMC,
    BCM2835_CLOCK_UART,
    BCM2835_CLOCK_ARM,
    BCM2835_CLOCK_CORE,
    BCM2835_CLOCK_V3D,
    BCM2835_CLOCK_H264,
    BCM2835_CLOCK_ISP,
    BCM2835_CLOCK_SDRAM,
    BCM2835_CLOCK_PIXEL,
    BCM2835_CLOCK_PWM,
};

extern void init_bcm2835_mailbox(ulong bcm2835_base);

extern void bcm2835_mailbox_property_init();
extern void bcm2835_mailbox_property_tag(u32 tag, ...);
extern u32 bcm2835_mailbox_property_process();
extern struct bcm2835_mailbox_property *bcm2835_mailbox_property_get(u32 tag);


/*
 * Framebuffer
 */
extern void init_bcm2835_framebuffer(ulong bcm2835_end);
extern void bcm2835_framebuffer_test();
extern int bcm2835_is_framebuffer_avail();
extern void bcm2835_get_framebuffer_info(void **f, int *w, int *h, int *d, int *p);


/*
 * Misc
 */
extern u32 bcm2835_get_board_model();
extern u32 bcm2835_get_board_revision();
extern u64 bcm2835_get_board_serial();
extern u32 bcm2835_get_firmware_version();
extern u64 bcm2835_get_mac_address();
extern u32 bcm2835_get_max_cpu_clock();
extern void bcm2835_maximize_cpu_clock();


#endif
