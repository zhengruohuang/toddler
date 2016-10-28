#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/int.h"
#include "hal/include/lib.h"
#include "hal/include/drv.h"


#define KEYBOARD_BUF_SIZE       (sizeof(ulong) * 2)
#define CHECK_FLAG(flags, bit)  (flags & (0x1 << bit))


static int char_count = 0;
static ulong buf1, buf2;


static void read_scan_code()
{
    ulong status = 0;
    
    char_count = 0;
    buf1 = buf2 = 0;
    status = (u32)io_in8(I8042_IO_STATUS);
    
    while (CHECK_FLAG(status, I8042_STATUS_UDATA) || CHECK_FLAG(status, I8042_STATUS_KDATA)) {
        if (CHECK_FLAG(status, I8042_STATUS_KDATA)) {
            //value = io_in8(I8042_IO_BUFFER);
            if (char_count < sizeof(ulong)) {
                buf1 <<= 8;
                buf1 |= io_in8(I8042_IO_BUFFER);
                char_count++;
            } else if (char_count < sizeof(ulong) * 2) {
                buf2 <<= 8;
                buf2 |= io_in8(I8042_IO_BUFFER);
                char_count++;
            }
        }
        
        status = io_in8(I8042_IO_STATUS);
    }
}

void init_keyboard()
{
    read_scan_code();
    kprintf("Keyboard initialized\n");
}

int keyboard_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    read_scan_code();
    
    kdi->interrupt.param0 = (ulong)char_count;
    kdi->interrupt.param1 = buf1;
    kdi->interrupt.param2 = buf2;
    
    return 1;
}
