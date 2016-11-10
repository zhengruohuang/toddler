#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static int do_ls(char *name)
{
    char buf[64];
    unsigned long id = open_path(name, 0);
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
    return err;
}

int ls(int argc, char **argv)
{
    int err = EOK;
    
    if (argc > 1) {
        int i;
        
        for (i = 1; i < argc; i++) {
            if (argc > 2) {
                kprintf("%s:\n", argv[i]);
            }
            
            err = do_ls(argv[i]);
            if (i < argc - 1) {
                kprintf("\n");
            }
            
            if (err != EOK) {
                break;
            }
        }
    }
    
    else {
    
        char *path = get_cwd();
        do_ls(path);
        free(path);
    }
    
    return err;
}
