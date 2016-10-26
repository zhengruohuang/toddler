#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"


int hello(int argc, char **argv)
{
    int i;
    
    kprintf("This is Toddler shell!\n");
    
    kprintf("Argument (%d): ", argc);
    for (i = 0; i < argc; i++) {
        if (i) {
            kprintf(", ");
        }
        
        kprintf(argv[i]);
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
    
    return EOK;
}
