#include "common/include/data.h"
#include "loader/include/periph/bcm2835.h"


#define DEFAULT_DEPTH   32

static u32 width = 0, height = 0;
static u32 depth = 0, pitch = 0;

static volatile u8 *fb;


void init_bcm2835_framebuffer(ulong bcm2835_end)
{
    struct bcm2835_mailbox_property *mp;
    
    // Detect display size
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_PHYSICAL_SIZE);
    bcm2835_mailbox_property_process();
    
    // Check if display exists
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_PHYSICAL_SIZE);
    if (!mp) {
        fb = NULL;
        return;
    }
    
    width = mp->data.buf32[0];
    height = mp->data.buf32[1];
    if (!width || !height) {
        width = height = 0;
        fb = NULL;
        return;
    }
    
    // Initialize the framebuffer
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_SET_PHYSICAL_SIZE, width, height);
    bcm2835_mailbox_property_tag(BCM2835_TAG_SET_VIRTUAL_SIZE, width, height);
    bcm2835_mailbox_property_tag(BCM2835_TAG_SET_DEPTH, DEFAULT_DEPTH);
    bcm2835_mailbox_property_tag(BCM2835_TAG_ALLOCATE_BUFFER);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_ALLOCATE_BUFFER);
    if (mp) {
        fb = (u8 *)(ulong)(mp->data.buf32[0] & bcm2835_end);
    }
    
    // Get framebuffer info
    bcm2835_mailbox_property_init();
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_PITCH);
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_DEPTH);
    bcm2835_mailbox_property_tag(BCM2835_TAG_GET_PHYSICAL_SIZE);
    bcm2835_mailbox_property_process();
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_PHYSICAL_SIZE);
    if (mp) {
        width = mp->data.buf32[0];
        height = mp->data.buf32[1];
    }
    
    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_DEPTH);
    if (mp) {
        depth = mp->data.buf32[0] >> 3;
    }

    mp = bcm2835_mailbox_property_get(BCM2835_TAG_GET_PITCH);
    if (mp) {
        pitch = mp->data.buf32[0];
    }
}

int bcm2835_is_framebuffer_avail()
{
    return fb ? 1 : 0;
}

void bcm2835_framebuffer_test()
{
    int yoffset = 0, offset = 0;
    int x, y, d;
    u8 color = 0x60;
    
    while (1) {
        yoffset = 0;
        for (y = 0; y < height; y++) {
            offset = yoffset;
            for (x = 0; x < width; x++) {
                for (d = 0; d < depth; d++) {
                    fb[offset + d] = color += 0x80;
                }
                
                offset += depth;
                color += 0x1;
            }
            yoffset += pitch;
            color += 0x1;
        }
    }
}

void bcm2835_get_framebuffer_info(void **f, int *w, int *h, int *d, int *p)
{
    if (!fb) {
        return;
    }
    
    if (f) *f = (void *)fb;
    if (w) *w = width;
    if (h) *h = height;
    if (d) *d = depth;
    if (p) *p = pitch;
}
