#ifndef __HAL_INCLUDE_DRV__
#define __HAL_INCLUDE_DRV__


#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/int.h"


/*
 * Keyboard
 */
// Keyboard IO ports
#define I8042_IO_BUFFER     0x60
#define I8042_IO_STATUS     0x64

// Keyboard interface bits
#define I8042_STATUS_KDATA  0   // keyboard data is in buffer (output buffer is empty) (bit 0)
#define I8042_STATUS_UDATA  1   // user data is in buffer (command buffer is empty) (bit 1)

// Reset CPU
#define I8042_CMD_RESET     0xFE

extern int keyboard_interrupt_handler(struct int_context *context, struct kernel_dispatch_info *kdi);


#endif
