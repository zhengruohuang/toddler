#include "common/include/data.h"
#include "loader/include/arg.h"
#include "loader/include/lib.h"
#include "loader/include/periph/bcm2835.h"


static volatile struct bcm2835_mailbox *mailbox0;

static volatile u32 mailbox_buf[8192] __attribute__((aligned(16)));
static volatile u32 mailbox_index;


void init_bcm2835_mailbox(ulong bcm2835_base)
{
    mailbox0 = (void *)(bcm2835_base + BCM2835_MAILBOX0_BASE);
}

static void mailbox0_write(int channel, u32 value)
{
    // Add the channel number into the lower 4 bits
    value &= ~(0xF);
    value |= (u32)channel;

    // Wait until the mailbox becomes available
    while ((mailbox0->status & BCM2835_MAILBOX_FULL) != 0);

    // Write the modified value + channel number into the write register
    mailbox0->write = value;
}

static u32 mailbox0_read(int channel)
{
    u32 value = -1;

    // Keep reading the register until the desired channel gives us a value
    while ((value & 0xF) != (u32)channel) {
        // Wait while the mailbox is empty
        while (mailbox0->status & BCM2835_MAILBOX_EMPTY);

        // Extract the value from the Read register of the mailbox
        // The value is actually in the upper 28 bits
        value = mailbox0->read;
    }

    // Return just the value (the upper 28-bits)
    return value >> 4;
}

void bcm2835_mailbox_property_init()
{
    /* Fill in the size on-the-fly */
    mailbox_buf[BCM2835_TAG_BUF_OFFSET_SIZE_] = 12;

    /* Process request (All other values are reserved!) */
    mailbox_buf[BCM2835_TAG_BUF_OFFSET_REQUEST_OR_RESPONSE] = 0;

    /* First available data slot */
    mailbox_index = 2;

    /* NULL tag to terminate tag list */
    mailbox_buf[mailbox_index] = 0;
}

void bcm2835_mailbox_property_tag(u32 tag, ...)
{
    va_list vl;
    va_start(vl, tag);

    mailbox_buf[mailbox_index++] = tag;

    switch (tag) {
    case BCM2835_TAG_GET_FIRMWARE_VERSION:
    case BCM2835_TAG_GET_BOARD_MODEL:
    case BCM2835_TAG_GET_BOARD_REVISION:
    case BCM2835_TAG_GET_BOARD_MAC_ADDRESS:
    case BCM2835_TAG_GET_BOARD_SERIAL:
    case BCM2835_TAG_GET_ARM_MEMORY:
    case BCM2835_TAG_GET_VC_MEMORY:
    case BCM2835_TAG_GET_DMA_CHANNELS:
        /* Provide an 8-byte buffer for the response */
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_index += 2;
        break;

    case BCM2835_TAG_GET_CLOCKS:
    case BCM2835_TAG_GET_COMMAND_LINE:
        /* Provide a 256-byte buffer */
        mailbox_buf[mailbox_index++] = 256;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_index += 256 >> 2;
        break;

    case BCM2835_TAG_ALLOCATE_BUFFER:
    case BCM2835_TAG_GET_MAX_CLOCK_RATE:
    case BCM2835_TAG_GET_MIN_CLOCK_RATE:
    case BCM2835_TAG_GET_CLOCK_RATE:
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_buf[mailbox_index++] = va_arg(vl, int);
        mailbox_buf[mailbox_index++] = 0;
        break;

    case BCM2835_TAG_SET_CLOCK_RATE:
        mailbox_buf[mailbox_index++] = 12;
        mailbox_buf[mailbox_index++] = 0; /* Request */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Clock ID */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Rate (in Hz) */
        mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Skip turbo setting if == 1 */
        break;

    case BCM2835_TAG_GET_PHYSICAL_SIZE:
    case BCM2835_TAG_SET_PHYSICAL_SIZE:
    case BCM2835_TAG_TEST_PHYSICAL_SIZE:
    case BCM2835_TAG_GET_VIRTUAL_SIZE:
    case BCM2835_TAG_SET_VIRTUAL_SIZE:
    case BCM2835_TAG_TEST_VIRTUAL_SIZE:
    case BCM2835_TAG_GET_VIRTUAL_OFFSET:
    case BCM2835_TAG_SET_VIRTUAL_OFFSET:
        mailbox_buf[mailbox_index++] = 8;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (
            (tag == BCM2835_TAG_SET_PHYSICAL_SIZE) ||
            (tag == BCM2835_TAG_SET_VIRTUAL_SIZE) ||
            (tag == BCM2835_TAG_SET_VIRTUAL_OFFSET) ||
            (tag == BCM2835_TAG_TEST_PHYSICAL_SIZE) ||
            (tag == BCM2835_TAG_TEST_VIRTUAL_SIZE)
      ) {
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Width */
            mailbox_buf[mailbox_index++] = va_arg(vl, int); /* Height */
        } else {
            mailbox_index += 2;
        }
        break;

    case BCM2835_TAG_GET_ALPHA_MODE:
    case BCM2835_TAG_SET_ALPHA_MODE:
    case BCM2835_TAG_GET_DEPTH:
    case BCM2835_TAG_SET_DEPTH:
    case BCM2835_TAG_GET_PIXEL_ORDER:
    case BCM2835_TAG_SET_PIXEL_ORDER:
    case BCM2835_TAG_GET_PITCH:
        mailbox_buf[mailbox_index++] = 4;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (
            (tag == BCM2835_TAG_SET_DEPTH) ||
            (tag == BCM2835_TAG_SET_PIXEL_ORDER) ||
            (tag == BCM2835_TAG_SET_ALPHA_MODE)
      ) {
            /* Colour Demailbox_bufh, bits-per-pixel \ Pixel Order State */
            mailbox_buf[mailbox_index++] = va_arg(vl, int);
        } else {
            mailbox_index += 1;
        }
        break;

    case BCM2835_TAG_GET_OVERSCAN:
    case BCM2835_TAG_SET_OVERSCAN:
        mailbox_buf[mailbox_index++] = 16;
        mailbox_buf[mailbox_index++] = 0; /* Request */

        if (tag == BCM2835_TAG_SET_OVERSCAN) {
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

u32 bcm2835_mailbox_property_process()
{
    u32 result;

    /* Fill in the size of the buffer */
    mailbox_buf[BCM2835_TAG_BUF_OFFSET_SIZE_] = (mailbox_index + 1) << 2;
    mailbox_buf[BCM2835_TAG_BUF_OFFSET_REQUEST_OR_RESPONSE] = 0;

    mailbox0_write(BCM2835_MAILBOX0_TAGS_ARM_TO_VC, (u32)mailbox_buf);
    result = mailbox0_read(BCM2835_MAILBOX0_TAGS_ARM_TO_VC);

    return result;
}

struct bcm2835_mailbox_property *bcm2835_mailbox_property_get(u32 tag)
{
    static struct bcm2835_mailbox_property property;
    volatile u32 *tag_buffer = NULL;

    property.tag = tag;

    /* Get the tag from the buffer. Start at the first tag position  */
    int index = 2;

    while (index < (mailbox_buf[BCM2835_TAG_BUF_OFFSET_SIZE_] >> 2)) {
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
    property.byte_length = tag_buffer[BCM2835_TAG_OFFSET_RESPONSE] & 0xFFFF;
    memcpy(property.data.buf8, (void *)&tag_buffer[BCM2835_TAG_OFFSET_VALUE], property.byte_length);

    return &property;
}
