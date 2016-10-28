#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "shell/include/shell.h"


int hello(int argc, char **argv)
{
    int i;

    kprintf("This is Toddler shell!\n");
    welcome();
    
    if (argc) {
        kprintf("Args (%d): ", argc);
        for (i = 0; i < argc; i++) {
            if (i) {
                kprintf(", ");
            }
            
            kprintf(argv[i]);
        }
        kprintf("\n");
    }
    
    return EOK;
}


int echo(int argc, char **argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
        if (i) {
            kprintf(" ");
        }
        
        kprintf(argv[i]);
    }
    kprintf("\n");
    
    return EOK;
}
