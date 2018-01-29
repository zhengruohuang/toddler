#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


#define BCM2835_GPIO_BASE   (0x200000ul)

// A mask to be able to clear the bits in the register before setting the value we require
#define BCM2835_GPIO_FUNC_MASK   (7)

// The GPIO Peripheral is described in section 6 of the BCM2835 Peripherals documentation.
// There are 54 general-purpose I/O (GPIO) lines split into two banks. All
// GPIO pins have at least two alternative functions within BCM. The
// alternate functions are usually peripheral IO and a single peripheral
// may appear in each bank to allow flexibility on the choice of IO voltage.
// Details of alternative functions are given in section 6.2. Alternative
// Function Assignments.
// The GPIO peripheral has three dedicated interrupt lines. These lines are
// triggered by the setting of bits in the event detect status register. Each
// bank has its’ own interrupt line with the third line shared between all bits.
// The Alternate function table also has the pull state (pull-up/pull-down)
// which is applied after a power down.
typedef volatile u32 bcm2835_reg_rw_t;
typedef volatile const u32 bcm2835_reg_ro_t;
typedef volatile u32 bcm2835_reg_wo_t;

struct bcm2835_gpio {
    bcm2835_reg_rw_t    GPFSEL0;
    bcm2835_reg_rw_t    GPFSEL1;
    bcm2835_reg_rw_t    GPFSEL2;
    bcm2835_reg_rw_t    GPFSEL3;
    bcm2835_reg_rw_t    GPFSEL4;
    bcm2835_reg_rw_t    GPFSEL5;
    bcm2835_reg_ro_t    reserved0;
    bcm2835_reg_wo_t    GPSET0;
    bcm2835_reg_wo_t    GPSET1;
    bcm2835_reg_ro_t    reserved1;
    bcm2835_reg_wo_t    GPCLR0;
    bcm2835_reg_wo_t    GPCLR1;
    bcm2835_reg_ro_t    reserved2;
    bcm2835_reg_wo_t    GPLEV0;
    bcm2835_reg_wo_t    GPLEV1;
    bcm2835_reg_ro_t    reserved3;
    bcm2835_reg_wo_t    GPEDS0;
    bcm2835_reg_wo_t    GPEDS1;
    bcm2835_reg_ro_t    reserved4;
    bcm2835_reg_wo_t    GPREN0;
    bcm2835_reg_wo_t    GPREN1;
    bcm2835_reg_ro_t    reserved5;
    bcm2835_reg_wo_t    GPFEN0;
    bcm2835_reg_wo_t    GPFEN1;
    bcm2835_reg_ro_t    reserved6;
    bcm2835_reg_wo_t    GPHEN0;
    bcm2835_reg_wo_t    GPHEN1;
    bcm2835_reg_ro_t    reserved7;
    bcm2835_reg_wo_t    GPLEN0;
    bcm2835_reg_wo_t    GPLEN1;
    bcm2835_reg_ro_t    reserved8;
    bcm2835_reg_wo_t    GPAREN0;
    bcm2835_reg_wo_t    GPAREN1;
    bcm2835_reg_ro_t    reserved9;
    bcm2835_reg_wo_t    GPAFEN0;
    bcm2835_reg_wo_t    GPAFEN1;
    bcm2835_reg_ro_t    reserved10;
    bcm2835_reg_wo_t    GPPUD;
    bcm2835_reg_wo_t    GPPUDCLK0;
    bcm2835_reg_wo_t    GPPUDCLK1;
    bcm2835_reg_ro_t    reserved11;
};


volatile struct bcm2835_gpio *gpio;


void init_bcm2835_gpio(ulong bcm2835_base)
{
    gpio = (void *)(bcm2835_base + BCM2835_GPIO_BASE);
}


static void gpio_set_pin_func(int pin, int func)
{
    bcm2835_reg_rw_t* fsel_reg = &((bcm2835_reg_rw_t *)gpio)[pin / 10];
    bcm2835_reg_rw_t fsel_copy = *fsel_reg;
    fsel_copy &= ~(BCM2835_GPIO_FUNC_MASK << (pin % 10 * 3));
    fsel_copy |= func << (pin % 10 * 3);
    *fsel_reg = fsel_copy;
}

void gpio_set_output(int pin)
{
    gpio_set_pin_func(pin, BCM2835_FUNC_OUTPUT);
}


void gpio_set_input(int pin)
{
    gpio_set_pin_func(pin, BCM2835_FUNC_INPUT);
}

static void gpio_set_high(int pin)
{
    switch (pin / 32) {
    case 0:
        gpio->GPSET0 = 1 << pin;
        break;
    case 1:
        gpio->GPSET1 = 1 << (pin - 32);
        break;
    default:
        break;
    }
}

static void gpio_set_low(int pin)
{
    switch (pin / 32) {
    case 0:
        gpio->GPCLR0 = 1 << pin;
        break;
    case 1:
        gpio->GPCLR1 = 1 << (pin - 32);
        break;
    default:
        break;
    }
}

int gpio_get(int pin)
{
    int result = BCM2835_GPIO_VAL_UNKNOWN;

    switch (pin / 32) {
    case 0:
        result = gpio->GPLEV0 & (1 << pin);
        break;
    case 1:
        result = gpio->GPLEV1 & (1 << (pin - 32));
        break;
    default:
        break;
    }

    if (result != BCM2835_GPIO_VAL_UNKNOWN && result) {
        result = BCM2835_GPIO_VAL_HI;
    }

    return result;
}

void gpio_set(int pin, int value)
{
    if (value == BCM2835_GPIO_VAL_LO || value == BCM2835_GPIO_VAL_OFF) {
        gpio_set_low(pin);
    } else if (value == BCM2835_GPIO_VAL_HI || value == BCM2835_GPIO_VAL_ON) {
        gpio_set_high(pin);
    }
}

void gpio_toggle(int pin)
{
    if (gpio_get(pin)) {
        gpio_set_low(pin);
    } else {
        gpio_set_high(pin);
    }
}


/*
 * Pull-up/down
 */
enum bcm2835_pud_ctrl {
    BCM2835_GPIO_PUD_OFF    = 0x00, // 0b00, Off ? disable pull-up/down
    BCM2835_GPIO_PUD_DOWN   = 0x01, // 0b01, Enable Pull Down control
    BCM2835_GPIO_PUD_UP     = 0x02  // 0b10, Enable Pull Up control
};

static void bcm2835_gpio_pud(const int pud)
{
    gpio->GPPUD = (u32)pud;
}

static void bcm2835_gpio_pudclk(const int pin, const int on)
{
    gpio->GPPUDCLK0 = (u32)((on ? 1 : 0) << pin);
}

void bcm2835_gpio_set_pud(const int pin, const int pud)
{
    bcm2835_gpio_pud(pud);
    bcm2835_delay(10);
    bcm2835_gpio_pudclk(pin, 1);
    bcm2835_delay(10);
    bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_pudclk(pin, 0);
}


/*
 * UART & PL011
 */
#define RPI_V2_GPIO_P1_08   14  // Version 2, Pin P1-08, defaults to ALT function 0 PL011_TXD
#define RPI_V2_GPIO_P1_10   15  // Version 2, Pin P1-10, defaults to ALT function 0 PL011_RXD

void bcm2835_gpio_enable_uart()
{
    // Setup GPIO 14 and 15 as alternative function 5 which is UART 1 TXD/RXD. These need to be set before enabling the UART
    gpio_set_pin_func(BCM2835_GPIO_PIN14, BCM2835_FUNC_ALT5);
    gpio_set_pin_func(BCM2835_GPIO_PIN15, BCM2835_FUNC_ALT5);
    
    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);
}

void bcm2835_gpio_enable_pl011()
{
    // Set the GPI0 pins to the Alt 0 function to enable PL011 access on them
    gpio_set_pin_func(RPI_V2_GPIO_P1_08, BCM2835_FUNC_ALT0);    // PL011_TXD
    gpio_set_pin_func(RPI_V2_GPIO_P1_10, BCM2835_FUNC_ALT0);    // PL011_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);
}


/*
 * LED
 */
#define LED_GPFSEL      GPFSEL4
#define LED_GPFBIT      21
#define LED_GPSET       GPSET1
#define LED_GPCLR       GPCLR1
#define LED_GPIO_BIT    15

void bcm2835_gpio_led_set()
{
    gpio->LED_GPSET = (1 << LED_GPIO_BIT);
}

void bcm2835_gpio_led_clear()
{
    gpio->LED_GPCLR = (1 << LED_GPIO_BIT);
}
