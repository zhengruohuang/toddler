#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static int do_rm(char *name)
{
    unsigned long id = open_path(name, 0);
//     kprintf("Open: %p\n", id);
    
    int err = kapi_urs_remove(id, 0);
    return err;
}

int rm(int argc, char **argv)
{
    int err = EOK;
    
    if (argc > 1) {
        int i;
        
        for (i = 1; i < argc; i++) {
            err = do_rm(argv[i]);
            if (err != EOK) {
                break;
            }
        }
    }
    
    return err;
}
