#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


u32 bcm2835_get_board_model()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_BOARD_MODEL);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_BOARD_MODEL);
    if (mp) {
        return mp->data.val32;
    }
    
    return 0;
}

u32 bcm2835_get_board_revision()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_BOARD_REVISION);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_BOARD_REVISION);
    if (mp) {
        return mp->data.val32;
    }
    
    return 0;
}

u64 bcm2835_get_board_serial()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_BOARD_SERIAL);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_BOARD_SERIAL);
    if (mp) {
        u64 serial = ((u64)mp->data.buf32[0] << 32) | (u64)mp->data.buf32[1];
        return serial;
    }
    
    return 0;
}

u32 bcm2835_get_firmware_version()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_FIRMWARE_VERSION);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_FIRMWARE_VERSION);
    if (mp) {
        return mp->data.val32;
    }
    
    return 0;
}

u64 bcm2835_get_mac_address()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_BOARD_MAC_ADDRESS);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_BOARD_MAC_ADDRESS);
    if (mp) {
        u64 mac =
            ((u64)mp->data.buf8[0] << 40) |
            ((u64)mp->data.buf8[1] << 32) |
            ((u64)mp->data.buf8[2] << 24) |
            ((u64)mp->data.buf8[3] << 16) |
            ((u64)mp->data.buf8[4] << 8)  |
            ((u64)mp->data.buf8[5] << 0);
        
        return mac;
    }
    
    return 0;
}

u32 bcm2835_get_max_cpu_clock()
{
    struct bcm2835_mailbox_property *mp;
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_MAX_CLOCK_RATE, BCM2835_CLOCK_ARM);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_MAX_CLOCK_RATE);
    if (mp) {
        return mp->data.buf32[1];
    }
    
    return 0;
}

void bcm2835_maximize_cpu_clock()
{
    u32 max_clock = bcm2835_get_max_cpu_clock();
    
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_SET_CLOCK_RATE, BCM2835_CLOCK_ARM, max_clock);
    bcm2835_mailbox_property_process();
}
