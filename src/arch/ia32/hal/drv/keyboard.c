#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/int.h"
#include "hal/include/lib.h"
#include "hal/include/drv.h"


static int check_flag(u32 flags, int bit)
{
    return flags & (0x1 << bit);
}

static ulong read_scan_code()
{
    u32 temp, value;
    
    // Clear all keyboard buffers (output and command buffers)
    do {
        temp = (u32)io_in8(I8042_IO_STATUS);
        
        if (check_flag(temp, I8042_STATUS_KDATA)) {
            // Empty keyboard data
            value = io_in8(I8042_IO_BUFFER);
        }
    } while (check_flag(temp, I8042_STATUS_UDATA));
    
    return value;
}


void init_keyboard()
{
    read_scan_code();
    kprintf("Keyboard initialized\n");
}

int keyboard_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    // Dispatch info
    kdi->interrupt.param0 = read_scan_code();
    
    return 1;
}
