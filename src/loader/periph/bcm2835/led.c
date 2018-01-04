#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


enum bcm2835_led_type {
    BCM2835_LED_ORIGINAL,
    BCM2835_LED_PLUS,
    BCM2835_LED_GPIO_VIRT,
    BCM2835_LED_UNKNOWN,
};


static enum bcm2835_led_type led_type = BCM2835_LED_UNKNOWN;


void init_bcm2835_led()
{
    u32 board_rev = bcm2835_get_board_revision();
    if ((board_rev == 0xa02082) || (board_rev == 0xa22082)) {
        led_type = BCM2835_LED_GPIO_VIRT;
    } else if (board_rev > 0x00000f) {
        led_type = BCM2835_LED_PLUS;
    }
    
    bcm2835_led_off();
}

void bcm2835_led_on()
{
    switch (led_type) {
    case BCM2835_LED_ORIGINAL:
        bcm2835_gpio_led_set();
        break;
    case BCM2835_LED_PLUS:
        bcm2835_gpio_led_clear();
        break;
    case BCM2835_LED_GPIO_VIRT:
    default:
        break;
    }
}

void bcm2835_led_off()
{
    switch (led_type) {
    case BCM2835_LED_ORIGINAL:
        bcm2835_gpio_led_clear();
        break;
    case BCM2835_LED_PLUS:
        bcm2835_gpio_led_set();
        break;
    case BCM2835_LED_GPIO_VIRT:
    default:
        break;
    }
}

void bcm2835_led_blink(int times, int interval_ms)
{
    int i;
    int us = (interval_ms ? interval_ms : 300) * 1000;
    
    for (i = 0; i < times || !times; i++) {
        bcm2835_delay(us);
        bcm2835_led_on();
        bcm2835_delay(us);
        bcm2835_led_off();
    }
}
