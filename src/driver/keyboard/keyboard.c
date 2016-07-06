#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "driver/include/keyboard.h"


static asmlinkage void keyboard_interrupt_handler(msg_t *msg)
{
    kprintf("Got a keyboard interrupt!\n");
    
    kapi_thread_exit(NULL);
}

void init_keyboard()
{
    // Register keyboard handler
    kapi_interrupt_reg(1, keyboard_interrupt_handler);
    kprintf("Keyboard handler registered\n");
}
