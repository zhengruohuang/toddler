#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


int echo(int argc, char **argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
        if (i) {
            kprintf(" ");
        }
        
        kprintf("%s", argv[i]);
    }
    kprintf("\n");
    
    return EOK;
}
