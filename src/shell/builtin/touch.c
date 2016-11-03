#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static int do_touch(char *name)
{
    char buf[] = "Hello world!\nFile touched!\n";
    size_t j;
    int err = EOK;
    
    unsigned long id = kapi_urs_open(name, 0);
    kprintf("Open: %p\n", id);
    
    if (id) {
        size_t s = 0;
        s = kapi_urs_write(id, buf, sizeof(buf));
        err = kapi_urs_close(id);
        kprintf("Closed: %p (%d)\n", id, err);
    }
    
    else {
        err = ENOENT;
    }
    
    return err;
}

int touch(int argc, char **argv)
{
    int err = EOK;
    
    if (argc > 1) {
        int i;
        
        for (i = 1; i < argc; i++) {
            err = do_touch(argv[i]);
            if (err != EOK) {
                break;
            }
        }
    }
    
    else {
        err = do_touch("ramfs://");
    }
    
    return err;
}
