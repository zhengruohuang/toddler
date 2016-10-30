#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
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
            
            kprintf("%s", argv[i]);
        }
        kprintf("\n");
    }
    
    return EOK;
}
