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
 * HelenOS
 */


#define PIC_PENDING_HIGH  4
#define PIC_PENDING_LOW   8

#define PIC_MASK_HIGH     5
#define PIC_MASK_LOW      9

#define PIC_ACK_HIGH      6
#define PIC_ACK_LOW       10


static volatile u32 *pic = NULL;

static int default_pic_vector = 0;
static int wired_to_vector_map[64];


static void enable_int(int num)
{
    if (num < 32) {
        pic[PIC_MASK_LOW] |= 1 << num;
    } else {
        pic[PIC_MASK_HIGH] |= 1 << (num - 32);
    }
}

static void disable_int(int num)
{
    if (num < 32) {
        pic[PIC_MASK_LOW] &= ~(1 << num);
    } else {
        pic[PIC_MASK_HIGH] &= ~(1 << (num - 32));
    }
}

static void ack(int num)
{
    if (num < 32) {
        pic[PIC_ACK_LOW] = 1 << num;
    } else {
        pic[PIC_ACK_HIGH] = 1 << (num - 32);
    }
}

static int get_pending()
{
    u32 pending;
    
    pending = pic[PIC_PENDING_LOW];
    if (pending != 0) {
        return fnzb32(pending);
    }
    
    pending = pic[PIC_PENDING_HIGH];
    if (pending != 0) {
        return fnzb32(pending) + 32;
    }
        
    return -1;
}

static int default_int_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    int wired = get_pending();
    panic("Unregistered pending IRQ: %d\n", wired);
    
    return INT_HANDLE_TYPE_KERNEL;
}

void heathrow_pic_eoi(int wired)
{
    ack(wired);
}

int heathrow_pic_register_wired(int wired, int_handler handler)
{
    if (wired >= 64) {
        return -1;
    }
    
    int vector = alloc_int_vector(handler);
    wired_to_vector_map[wired] = vector;
    
    return vector;
}

int heathrow_pic_get_vector()
{
    int wired = get_pending();
    
    if (wired_to_vector_map[wired]) {
        return wired_to_vector_map[wired];
    } else {
        return default_pic_vector;
    }
}

void start_heathrow_pic()
{
    // Register the default handler
    default_pic_vector = alloc_int_vector(default_int_handler);
    
    // Enable all interrupts
    pic[PIC_MASK_LOW] = 0xffffffff;
    pic[PIC_MASK_HIGH] = 0xffffffff;
}

void init_heathrow_pic()
{
    struct boot_parameters *bp = get_bootparam();
    pic = (void *)bp->int_ctrl_addr;
    
    // Disable all interrupts
    pic[PIC_MASK_LOW] = 0;
    pic[PIC_MASK_HIGH] = 0;
    
    // Clear the wired to vector table
    int i;
    for (i = 0; i < 64; i++) {
        wired_to_vector_map[i] = 0;
    }
}
