#include "periph.h"
#include "loader.h"


/*
 * Timer
 */
static volatile struct sys_timer *sys_timer;

static void init_timer()
{
    sys_timer = (struct sys_timer *)RPI_SYSTIMER_BASE;
}

void wait(int us)
{
    volatile u32 ts = sys_timer->counter_lo;
    while (sys_timer->counter_lo - ts < (u32)us);
}


/*
 * GPIO
 */
static volatile rpi_gpio_t *rpi_gpio;

static void init_gpio()
{
    // Assign the address of the GPIO peripheral (Using ARM Physical Address)
    rpi_gpio = (rpi_gpio_t *)GPIO_BASE;
}

static void gpio_set_pin_func(rpi_gpio_pin_t pin, rpi_gpio_alt_function_t func)
{
    rpi_reg_rw_t* fsel_reg = &((rpi_reg_rw_t *)rpi_gpio)[pin / 10];
    rpi_reg_rw_t fsel_copy = *fsel_reg;
    fsel_copy &= ~(FS_MASK << (pin % 10 * 3));
    fsel_copy |= func << (pin % 10 * 3);
    *fsel_reg = fsel_copy;
}

void gpio_set_output(rpi_gpio_pin_t pin)
{
    gpio_set_pin_func(pin, FS_OUTPUT);
}


void gpio_set_input(rpi_gpio_pin_t pin)
{
    gpio_set_pin_func(pin, FS_INPUT);
}

static void gpio_set_high(rpi_gpio_pin_t pin)
{
    switch (pin / 32) {
    case 0:
        rpi_gpio->GPSET0 = 1 << pin;
        break;
    case 1:
        rpi_gpio->GPSET1 = 1 << (pin - 32);
        break;
    default:
        break;
    }
}

static void gpio_set_low(rpi_gpio_pin_t pin)
{
    switch (pin / 32) {
    case 0:
        rpi_gpio->GPCLR0 = 1 << pin;
        break;
    case 1:
        rpi_gpio->GPCLR1 = 1 << (pin - 32);
        break;
    default:
        break;
    }
}

rpi_gpio_value_t gpio_get(rpi_gpio_pin_t pin)
{
    rpi_gpio_value_t result = RPI_IO_UNKNOWN;

    switch (pin / 32) {
    case 0:
        result = rpi_gpio->GPLEV0 & (1 << pin);
        break;
    case 1:
        result = rpi_gpio->GPLEV1 & (1 << (pin - 32));
        break;
    default:
        break;
    }

    if (result != RPI_IO_UNKNOWN && result) {
        result = RPI_IO_HI;
    }

    return result;
}

void gpio_set(rpi_gpio_pin_t pin, rpi_gpio_value_t value)
{
    if (value == RPI_IO_LO || value == RPI_IO_OFF) {
        gpio_set_low(pin);
    } else if (value == RPI_IO_HI || value == RPI_IO_ON) {
        gpio_set_high(pin);
    }
}

void gpio_toggle(rpi_gpio_pin_t pin)
{
    if (gpio_get(pin)) {
        gpio_set_low(pin);
    } else {
        gpio_set_high(pin);
    }
}


/*
 * LED
 */
static void init_led()
{
    // Write 1 to the GPIO16 init nibble in the Function Select 1 GPIO peripheral register to enable GPIO16 as an output
    rpi_gpio->LED_GPFSEL |= (1 << LED_GPFBIT);
    led_blink(2, 500);
}

void led_on()
{
    // Set the LED GPIO pin high (Turn OK LED off for original Pi, and on for plus models)
    rpi_gpio->LED_GPCLR = (1 << LED_GPIO_BIT);
}

void led_off()
{
    // Set the LED GPIO pin low (Turn OK LED on for original Pi, and off for plus models)
    rpi_gpio->LED_GPSET = (1 << LED_GPIO_BIT);
}

void led_blink(int times, int interval_ms)
{
    int i;
    int us = (interval_ms ? interval_ms : 300) * 1000;
    
    for (i = 0; i < times || !times; i++) {
        wait(us);
        led_on();
        wait(us);
        led_off();
    }
}


/*
 * UART
 */
static aux_t *auxillary = (aux_t *)AUX_BASE;

static void init_uart(int baud, int bits)
{
    volatile int i;

    // As this is a mini uart the configuration is complete! Now just enable the uart. Note from the documentation in section 2.1.1 of the ARM peripherals manual:
    // If the enable bits are clear you will have no access to a peripheral. You can not even read or write the registers
    auxillary->ENABLES = AUX_ENA_MINIUART;

    // Disable interrupts for now
    // auxillary->IRQ &= ~AUX_IRQ_MU;
    auxillary->MU_IER = 0;

    // Disable flow control,enable transmitter and receiver!
    auxillary->MU_CNTL = 0;

    // Decide between seven or eight-bit mode */
    if (bits == 8) {
        auxillary->MU_LCR = AUX_MULCR_8BIT_MODE;
    } else {
        auxillary->MU_LCR = 0;
    }

    auxillary->MU_MCR = 0;

    // Disable all interrupts from MU and clear the fifos
    auxillary->MU_IER = 0;
    auxillary->MU_IIR = 0xC6;

    // Transposed calculation from Section 2.2.1 of the ARM peripherals manual
    auxillary->MU_BAUD = (UART_FREQ / (8 * baud)) - 1;

    // Setup GPIO 14 and 15 as alternative function 5 which is UART 1 TXD/RXD. These need to be set before enabling the UART
    gpio_set_pin_func(RPI_GPIO14, FS_ALT5);
    gpio_set_pin_func(RPI_GPIO15, FS_ALT5);

    rpi_gpio->GPPUD = 0;
    wait(150);
    rpi_gpio->GPPUDCLK0 = 1 << 14;
    wait(150);
    rpi_gpio->GPPUDCLK0 = 0;

    // Disable flow control,enable transmitter and receiver!
    auxillary->MU_CNTL = AUX_MUCNTL_TX_ENABLE;
}

void uart_write(char ch)
{
    // Wait until the UART has an empty space in the FIFO
    while ((auxillary->MU_LSR & AUX_MULSR_TX_EMPTY) == 0);

    // Write the character to the FIFO for transmission
    auxillary->MU_IO = ch;
}


/*
 * Mailbox
 */
static volatile mailbox_t *mailbox0;
static volatile u32 mailbox_buf[8192] __attribute__((aligned(16)));
static volatile u32 mailbox_index;

static void memcpy(void *dest, void *src, u32 count)
{
    u32 i;
    
    unsigned char *s = (unsigned char *)src;
    unsigned char *d = (unsigned char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
    }
    
}

static void init_mailbox()
{
    mailbox0 = (mailbox_t *)RPI_MAILBOX0_BASE;
}

void mailbox0_write(mailbox0_channel_t channel, u32 value)
{
    // Add the channel number into the lower 4 bits
    value &= ~(0xF);
    value |= (u32)channel;

    // Wait until the mailbox becomes available and then write to the mailbox channel
    while ((mailbox0->Status & ARM_MS_FULL) != 0);

    // Write the modified value + channel number into the write register
    mailbox0->Write = value;
}

u32 mailbox0_read(mailbox0_channel_t channel)
{
    u32 value = -1;

    // Keep reading the register until the desired channel gives us a value
    while ((value & 0xF) != (u32)channel) {
        /* Wait while the mailbox is emmailbox_bufy because otherwise there's no value to read! */
        while (mailbox0->Status & ARM_MS_EMPTY);

        /* Extract the value from the Read register of the mailbox. The value is actually in the upper 28 bits */
        value = mailbox0->Read;
    }

    /* Return just the value (the upper 28-bits) */
    return value >> 4;
}

void property_init()
{
    /* Fill in the size on-the-fly */
    mailbox_buf[PT_OSIZE] = 12;

    /* Process request (All other values are reserved!) */
    mailbox_buf[PT_OREQUEST_OR_RESPONSE] = 0;

    /* First available data slot */
    mailbox_index = 2;

    /* NULL tag to terminate tag list */
    mailbox_buf[mailbox_index] = 0;
}

asmlinkage void property_tag(rpi_mailbox_tag_t tag, ...)
{
    va_list vl;
    va_start(vl, tag);

    mailbox_buf[mailbox_index++] = tag;

    switch (tag) {
    case TAG_GET_FIRMWARE_VERSION:
    case TAG_GET_BOARD_MODEL:
    case TAG_GET_BOARD_REVISION:
    case TAG_GET_BOARD_MAC_ADDRESS:
    case TAG_GET_BOARD_SERIAL:
    case TAG_GET_ARM_MEMORY:
    case TAG_GET_VC_MEMORY:
    case TAG_GET_DMA_CHANNELS:
        /* Provide an 8-byte buffer for the response */
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_index += 2;
        break;

    case TAG_GET_CLOCKS:
    case TAG_GET_COMMAND_LINE:
        /* Provide a 256-byte buffer */
        mailbox_buf[mailbox_index++] = 256;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_index += 256 >> 2;
        break;

    case TAG_ALLOCATE_BUFFER:
    case TAG_GET_MAX_CLOCK_RATE:
    case TAG_GET_MIN_CLOCK_RATE:
    case TAG_GET_CLOCK_RATE:
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_buf[mailbox_index++] = va_arg(vl, int);
        mailbox_buf[mailbox_index++] = 0;
        break;

    case TAG_SET_CLOCK_RATE:
        mailbox_buf[mailbox_index++] = 12;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Clock ID */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Rate (in Hz) */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Skip turbo setting if == 1 */
        break;

    case TAG_GET_PHYSICAL_SIZE:
    case TAG_SET_PHYSICAL_SIZE:
    case TAG_TEST_PHYSICAL_SIZE:
    case TAG_GET_VIRTUAL_SIZE:
    case TAG_SET_VIRTUAL_SIZE:
    case TAG_TEST_VIRTUAL_SIZE:
    case TAG_GET_VIRTUAL_OFFSET:
    case TAG_SET_VIRTUAL_OFFSET:
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (
            (tag == TAG_SET_PHYSICAL_SIZE) ||
            (tag == TAG_SET_VIRTUAL_SIZE) ||
            (tag == TAG_SET_VIRTUAL_OFFSET) ||
            (tag == TAG_TEST_PHYSICAL_SIZE) ||
            (tag == TAG_TEST_VIRTUAL_SIZE)
      ) {
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Width */
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Height */
        } else {
            mailbox_index += 2;
        }
        break;

    case TAG_GET_ALPHA_MODE:
    case TAG_SET_ALPHA_MODE:
    case TAG_GET_DEPTH:
    case TAG_SET_DEPTH:
    case TAG_GET_PIXEL_ORDER:
    case TAG_SET_PIXEL_ORDER:
    case TAG_GET_PITCH:
        mailbox_buf[mailbox_index++] = 4;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (
            (tag == TAG_SET_DEPTH) ||
            (tag == TAG_SET_PIXEL_ORDER) ||
            (tag == TAG_SET_ALPHA_MODE)
      ) {
            /* Colour Demailbox_bufh, bits-per-pixel \ Pixel Order State */
            mailbox_buf[mailbox_index++] = va_arg(vl, int);
        } else {
            mailbox_index += 1;
        }
        break;

    case TAG_GET_OVERSCAN:
    case TAG_SET_OVERSCAN:
        mailbox_buf[mailbox_index++] = 16;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (tag == TAG_SET_OVERSCAN) {
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Top pixels */
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Bottom pixels */
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Left pixels */
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Right pixels */
        } else {
            mailbox_index += 4;
        }
        break;

    default:
        /* Unsupported tags, just remove the tag from the list */
        mailbox_index--;
        break;
    }

    /* Make sure the tags are 0 terminated to end the list and update the buffer size */
    mailbox_buf[mailbox_index] = 0;

    va_end(vl);
}

u32 property_process()
{
    u32 result;

    /* Fill in the size of the buffer */
    mailbox_buf[PT_OSIZE] = (mailbox_index + 1) << 2;
    mailbox_buf[PT_OREQUEST_OR_RESPONSE] = 0;

    mailbox0_write(MB0_TAGS_ARM_TO_VC, (u32)mailbox_buf);
    result = mailbox0_read(MB0_TAGS_ARM_TO_VC);

    return result;
}

rpi_mailbox_property_t *property_get(rpi_mailbox_tag_t tag)
{
    static rpi_mailbox_property_t property;
    volatile u32 *tag_buffer = NULL;

    property.tag = tag;

    /* Get the tag from the buffer. Start at the first tag position  */
    int index = 2;

    while (index < (mailbox_buf[PT_OSIZE] >> 2)) {
        /* printf("Test Tag: [%d] %8.8X\n", index, mailbox_buf[index]); */
        if (mailbox_buf[index] == tag) {
            tag_buffer = &mailbox_buf[index];
            break;
        }

        /* Progress to the next tag if we haven't yet discovered the tag */
        index += (mailbox_buf[index + 1] >> 2) + 3;
    }

    /* Return NULL of the property tag cannot be found in the buffer */
    if (!tag_buffer) {
        return NULL;
    }

    /* Return the required data */
    property.byte_length = tag_buffer[T_ORESPONSE] & 0xFFFF;
    memcpy(property.data.buffer_8, (void *)&tag_buffer[T_OVALUE], property.byte_length);

    return &property;
}


/*
 * Clock frequency
 */
static void maximize_clock()
{
    rpi_mailbox_property_t *mp;
    
    property_init();
    property_tag(TAG_GET_MAX_CLOCK_RATE, TAG_CLOCK_ARM);
    property_process();
    
    mp = property_get(TAG_GET_MAX_CLOCK_RATE);
    if (mp) {
        property_init();
        property_tag(TAG_SET_CLOCK_RATE, TAG_CLOCK_ARM, mp->data.buffer_32[1]);
        property_process();
    }
}


/*
 * Framebuffer
 */
#include "font.h"

static u32 width = 640, height = 480;
static u32 bpp = 24, pitch = 0;
static volatile u8 *fb;

static u32 chars_per_row = 0, chars_per_col = 0;

static int init_display()
{
    rpi_mailbox_property_t *mp;
    
    // Get display size
    property_init();
    property_tag(TAG_GET_PHYSICAL_SIZE);
    property_process();
    
    mp = property_get(TAG_GET_PHYSICAL_SIZE);
    if (mp) {
        width = mp->data.buffer_32[0];
        height = mp->data.buffer_32[1];
        
        return 1;
    } else {
        return 0;
    }
}

static void init_fb()
{
    rpi_mailbox_property_t* mp;
    
    // Init framebuffer
    property_init();
    property_tag(TAG_ALLOCATE_BUFFER);
    property_tag(TAG_SET_PHYSICAL_SIZE, width, height);
    property_tag(TAG_SET_VIRTUAL_SIZE, width, height);
    property_tag(TAG_SET_DEPTH, bpp);
    property_tag(TAG_GET_PITCH);
    property_tag(TAG_GET_PHYSICAL_SIZE);
    property_tag(TAG_GET_DEPTH);
    property_process();
    
    mp = property_get(TAG_GET_PHYSICAL_SIZE);
    if (mp) {
        width = mp->data.buffer_32[0];
        height = mp->data.buffer_32[1];
    }

    mp = property_get(TAG_GET_DEPTH);
    if (mp) {
        bpp = mp->data.buffer_32[0] >> 3;
    }

    mp = property_get(TAG_GET_PITCH);
    if (mp) {
        pitch = mp->data.buffer_32[0];
    }

    mp = property_get(TAG_ALLOCATE_BUFFER);
    if (mp) {
        fb = (u8 *)(mp->data.buffer_32[0] & 0x3FFFFFFF);
    }
    
    // Init font
    chars_per_row = width / FONT_WIDTH;
    chars_per_col = height / FONT_HEIGHT;
}

static void fb_test()
{
    u8 yoffset = 0, offset = 0;
    u8 x, y, d;
    u8 color = 0x60;
    
    while (1) {
        yoffset = 0;
        for (y = 0; y < height; y++) {
            offset = yoffset;
            for (x = 0; x < width; x++) {
                for (d = 0; d < bpp; d++) {
                    fb[offset + d] = color;
                }
                
                offset += bpp;
            }
            yoffset += pitch;
            color += 0x3;
        }
    }
}

static void fb_draw_char(char ch, u32 line, u32 col)
{
    u32 offset = FONT_HEIGHT * line * pitch + FONT_WIDTH * col * bpp;
    u32 offset_x;
    
    u8 *font = vga_font[ch < 0x20 ? 0 : ch - 0x20];
    u8 cur_map;
    
    int i, j;
    for (i = 0; i < FONT_HEIGHT; i++) {
        cur_map = font[i];
        offset_x = offset;
        for (j = 0; j < FONT_WIDTH; j++) {
            if (cur_map & (0x1 << (FONT_WIDTH - 1 - j))) {
                fb[offset_x + 0] = 0xc0;
                fb[offset_x + 1] = 0xc0;
                fb[offset_x + 2] = 0xc0;
                fb[offset_x + 3] = 0xc0;
            } else {
                fb[offset_x + 0] = 0;
                fb[offset_x + 1] = 0;
                fb[offset_x + 2] = 0;
                fb[offset_x + 3] = 0;
            }
            
            offset_x += bpp;
        }
        
        offset += pitch;
    }
}

static void fb_update_cursor()
{
    if (chars_per_row && boot_param.cursor_col >= chars_per_row) {
        boot_param.cursor_row++;
        boot_param.cursor_col = 0;
    }
    
    if (chars_per_col && boot_param.cursor_row >= chars_per_col) {
        u32 move_y = (boot_param.cursor_row - chars_per_col + 1) * FONT_HEIGHT;
        u32 x, y, d, of_src, of_dest;
        
        u32 offset_src = move_y * pitch;
        u32 offset_dest = 0;
        
        for (y = move_y; y < height; y++) {
            of_src = offset_src;
            of_dest = offset_dest;
            
            for (x = 0; x < width; x++) {
                for (d = 0; d < bpp; d++) {
                    fb[of_dest + d] = fb[of_src + d];
                }
                
                of_src += bpp;
                of_dest += bpp;
            }
            
            offset_src += pitch;
            offset_dest += pitch;
        }
        
        boot_param.cursor_row = chars_per_col - 1;
    }
}


/*
 * Print
 */
static int fb_enabled = 0;

static void init_print()
{
    if (init_display()) {
        init_fb();
        fb_enabled = 1;
        led_blink(5, 500);
        
        boot_param.video_mode = VIDEO_FRAMEBUFFER;
        boot_param.cursor_row = 0;
        boot_param.cursor_col = 0;
        boot_param.framebuffer_addr = (ulong)fb;
        boot_param.res_x = width;
        boot_param.res_y = height;
        boot_param.bytes_per_pixel = bpp;
        boot_param.bytes_per_line = pitch;
        
        led_blink(5, 500);
    } else {
        init_uart(115200, 8);
        fb_enabled = 0;
        
        boot_param.video_mode = VIDEO_UART;
        boot_param.cursor_row = 0;
        boot_param.cursor_col = 0;
        boot_param.framebuffer_addr = 0;
        boot_param.res_x = 0;
        boot_param.res_y = 0;
        boot_param.bytes_per_pixel = 0;
        boot_param.bytes_per_line = 0;
    }
}

static void print_char(char ch)
{
    if (fb_enabled) {
        switch (ch) {
        case '\n':
        case '\r':
            boot_param.cursor_row++;
            boot_param.cursor_col = 0;
            break;
        case '\t':
            boot_param.cursor_col /= 8;
            boot_param.cursor_col = (boot_param.cursor_col + 1) * 8;
            break;
        default:
            fb_draw_char(ch, boot_param.cursor_row, boot_param.cursor_col);
            boot_param.cursor_col++;
            break;
        }
        
        fb_update_cursor();
    } else {
        uart_write(ch);
    }
    
}

static void print_string(char *str)
{
    char *s = str;
    
    while (*s) {
        print_char(*s);
        s++;
    }
}

static void uint_div(u32 a, u32 b, u32 *qout, u32 *rout)
{
    u32 q = 0, r = a;
    
    while (r >= b) {
        q++;
        r -= b;
    }
    
    if (qout) {
        *qout = q;
    }
    
    if (rout) {
        *rout = r;
    }
}

static void print_num(char fmt, u32 num)
{
    int i;
    u32 value;
    int started = 0;
    
    switch (fmt) {
    case 'b':
        for (i = 31; i >= 0; i--) {
            print_char(num & (0x1 << i) ? '1' : '0');
        }
        break;
    case 'x':
        print_string("0x");
    case 'h':
        if (!num) {
            print_char('0');
        } else {
            for (i = 0; i < sizeof(u32) * 8; i += 4) {
                value = (num << i) >> 28;
                if (value) {
                    started = 1;
                }
                if (started) {
                    print_char(value > 9 ? (u8)(value + 0x30 + 0x27) : (u8)(value + 0x30));
                }
            }
        }
        break;
    case 'd':
        if (num & (0x1 << 31)) {
            print_char('-');
            num = ~num + 1;
        }
    case 'u':
        if (!num) {
            print_char('0');
            break;
        } else {
            int digits = 10;
            u32 dividers[] = { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };
            u32 q, r;
            
            for (i = 0; i < digits; i++) {
                uint_div(num, dividers[i], &q, &r);
                
                if (q) {
                    started = 1;
                }
                
                if (started) {
                    print_char('0' + q);
                }
                
                num = r;
            }
        }
        break;
    default:
        print_string("__unknown_format_");
        print_char(fmt);
        print_string("__");
        break;
    }
}

asmlinkage void lprintf(char *fmt, ...)
{
    char *s = fmt;
    char token = 0;
    
    char *va_str;
    char va_ch;
    u32 va_u32;
    
    va_list va;
    va_start(va, fmt);
    
    while (*s) {
        switch (*s) {
        case '%':
            token = *(s + 1);
            switch (token) {
            case 'b':
            case 'd':
            case 'u':
            case 'h':
            case 'x':
                va_u32 = va_arg(va, u32);
                print_num(token, va_u32);
                s++;
                break;
            case 's':
                va_str = va_arg(va, char *);
                print_string(va_str);
                s++;
                break;
            case 'c':
                va_ch = va_arg(va, char);
                print_char(va_ch);
                s++;
                break;
            case '%':
                s++;
            default:
                print_char(token);
            }
            
            break;
        default:
            print_char(*s);
            break;
        }
        
        s++;
    }
    
    va_end(va);
}

static void print_device_info()
{
    rpi_mailbox_property_t *mp;
    
    print_string("Peripherals initialized... device info:\n");
    
    property_init();
    property_tag(TAG_GET_BOARD_MODEL);
    property_tag(TAG_GET_BOARD_REVISION);
    property_tag(TAG_GET_FIRMWARE_VERSION);
    property_tag(TAG_GET_BOARD_MAC_ADDRESS);
    property_tag(TAG_GET_BOARD_SERIAL);
    property_tag(TAG_GET_MAX_CLOCK_RATE, TAG_CLOCK_ARM);
    property_process();

    mp = property_get(TAG_GET_BOARD_MODEL);
    if (mp) {
        lprintf("\tBoard Model: %u\n", mp->data.value_32);
    } else {
        lprintf("\tBoard Model: NULL\n");
    }

    mp = property_get(TAG_GET_BOARD_REVISION);
    if (mp) {
        lprintf("\tBoard Revision: %u\n", mp->data.value_32);
    } else {
        lprintf("\tBoard Revision: NULL\n");
    }

    mp = property_get(TAG_GET_FIRMWARE_VERSION);
    if (mp) {
        lprintf("\tFirmware Version: %u\n", mp->data.value_32);
    } else {
        lprintf("\tFirmware Version: NULL\n");
    }

    mp = property_get(TAG_GET_BOARD_MAC_ADDRESS);
    if (mp) {
        lprintf("\tMAC Address: %h:%h:%h:%h:%h:%h\n",
               mp->data.buffer_8[0], mp->data.buffer_8[1], mp->data.buffer_8[2],
               mp->data.buffer_8[3], mp->data.buffer_8[4], mp->data.buffer_8[5]);
    } else {
        lprintf("\tMAC Address: NULL\n");
    }

    mp = property_get(TAG_GET_BOARD_SERIAL);
    if (mp) {
        lprintf("\tSerial Number: %h.%h\n", mp->data.buffer_32[0], mp->data.buffer_32[1]);
    } else {
        lprintf("\tSerial Number: NULL\n");
    }

    mp = property_get(TAG_GET_MAX_CLOCK_RATE);
    if (mp) {
        lprintf("\tMaximum ARM Clock Rate: %u Hz\n", mp->data.buffer_32[1]);
    } else {
        lprintf("\tMaximum ARM Clock Rate: NULL\n");
    }
    
    lprintf("\tFramebuffer: %s", fb_enabled ? "enabled" : "disabled");
    if (fb_enabled) {
        lprintf(" @ %x, width: %d, height: %d, depth: %d, pitch: %d\n", (u32)fb, width, height, bpp, pitch);
    } else {
        lprintf("\n");
    }
}


/*
 * Initialization
 */
void init_periph()
{
    init_timer();
    init_gpio();
    init_led();
    init_mailbox();
    maximize_clock();
    init_print();
    
    print_device_info();
    
    lprintf("printf test\n");
    lprintf("\tprintf dec: %u\n", 12345);
    lprintf("\tprintf dec: %d\n", 12345);
    lprintf("\tprintf neg: %d\n", -12345);
    lprintf("\tprintf hex: %x\n", 0x12345);
    lprintf("\tprintf bin: %b\n", 0x12345);
    
//     for (i = 0; i < 1024; i++) {
//         lprintf("Scrolling test: %d\n", i);
//     }
}
