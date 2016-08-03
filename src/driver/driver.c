#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "driver/include/keyboard.h"


int main(int argc, char *argv[])
{
    kprintf("Toddler driver process started!\n");
    
    // Initialize drivers
    init_keyboard();
    
    // Register KAPI handlers
    //kapi_init();
    //kprintf("Driver KAPI handlers initialized!\n");
    
    // Init done
    kapi_process_started(0);
    
    // Done
    do {
        syscall_yield();
    } while (1);
    
    return 0;
}
