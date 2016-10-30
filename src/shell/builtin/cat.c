#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static int do_cat(char *name)
{
    char buf[64 + 1];
    size_t j;
    int err = EOK;
    
    unsigned long id = kapi_urs_open(name, 0);
    kprintf("Open: %p\n", id);
    
    if (id) {
        size_t s = 0;
        do {
            s = kapi_urs_read(id, buf, sizeof(buf) - 1);
            if (s) {
                for (j = 0; j < s; j++) {
                    if (buf[j] == '%') {
                        buf[j] = '#';
                    }
                }
                buf[s] = '\0';
                kprintf("%s", buf);
            }
        } while (s);
        kprintf("\n");
        
        err = kapi_urs_close(id);
        kprintf("Closed: %p (%d)\n", id, err);
    }
    
    else {
        err = ENOENT;
    }
    
    return err;
}

int cat(int argc, char **argv)
{
    int err = EOK;
    
    if (argc > 1) {
        int i;
        
        for (i = 1; i < argc; i++) {
            err = do_cat(argv[i]);
            if (err != EOK) {
                break;
            }
        }
    }
    
    else {
        err = do_cat("coreimg://init.py");
    }
    
    return err;
}
