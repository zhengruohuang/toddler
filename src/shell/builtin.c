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

int ls(int argc, char **argv)
{
    char buf[64];
    unsigned long id = kapi_urs_open("coreimg://", 0);
    kprintf("Open: %p\n", id);
    
    int last = 0;
    do {
        last = kapi_urs_list(id, buf, sizeof(buf));
        if (!last) {
            kprintf("%s ", buf);
        }
    } while (!last);
    kprintf("\n");
    
    int err = kapi_urs_close(id);
    kprintf("Closed: %p (%d)\n", id, err);
}
