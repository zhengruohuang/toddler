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

int touch2(int argc, char **argv)
{
    int err = EOK;
    
    char *dir = NULL;
    char *name = NULL;
    if (argc > 2) {
        dir = argv[1];
        name = argv[2];
    } else {
        dir = "ramfs://";
        name = "test.txt";
    }
    
    // Create the node
    unsigned long id = kapi_urs_open(dir, 0);
    kprintf("Dir open: %p\n", id);
    if (!id) {
        err = ENOENT;
        return err;
    }
    
    err = kapi_urs_create(id, name, ucreate_node, 0, NULL);
    kprintf("Node create: %d\n", err);
    if (err) {
        return err;
    }
    
    err = kapi_urs_close(id);
    kprintf("Dir close: %p (%d)\n", id, err);
    if (err) {
        return err;
    }
    
    // Write something to the file
    
    
    return err;
}
