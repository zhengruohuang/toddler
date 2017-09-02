#include "common/include/data.h"
#include "hal/include/lib.h"
#include "hal/include/periph.h"


#define PIC_PENDING_HIGH  4
#define PIC_PENDING_LOW   8

#define PIC_MASK_HIGH     5
#define PIC_MASK_LOW      9

#define PIC_ACK_HIGH      6
#define PIC_ACK_LOW       10


static volatile u32 *pic = NULL;


void pic_enable_int(int num)
{
    if (num < 32) {
        pic[PIC_MASK_LOW] |= 1 << num;
    } else {
        pic[PIC_MASK_HIGH] |= 1 << (num - 32);
    }
}

void pic_disable_int(int num)
{
    if (num < 32) {
        pic[PIC_MASK_LOW] &= ~(1 << num);
    } else {
        pic[PIC_MASK_HIGH] &= ~(1 << (num - 32));
    }
}

void pic_ack(int num)
{
    if (num < 32) {
        pic[PIC_ACK_LOW] = 1 << num;
    } else {
        pic[PIC_ACK_HIGH] = 1 << (num - 32);
    }
}

int pic_get_pending()
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

void init_pic()
{
    //pic = (u32 *) km_map(base, size, PAGE_WRITE | PAGE_NOT_CACHEABLE);
}
