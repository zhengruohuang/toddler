#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static char touch_content[] = "Hello world!\nFile touched!";


static int touch_file(unsigned long id)
{
    int err = EOK;
    
    if (id) {
        size_t s = 0;
        s = kapi_urs_write(id, touch_content, sizeof(touch_content));
        err = kapi_urs_close(id);
//         kprintf("Closed: %p (%d)\n", id, err);
    }
    
    else {
        err = ENOENT;
    }
    
    return err;
}

static int create_file(char *name)
{
    int err = EOK;
    unsigned long id = 0;
    
    int i;
    int name_idx = 0;
    char *dir = NULL;
    char *file = NULL;
    
    // Find out dir name and file name
    for (i = strlen(name) - 1; i >= 0; i--) {
        if (name[i] == '/') {
            break;
        }
    }
    
    if (!i) {
        return ENOENT;
    }
    
    name_idx = i + 1;
    dir = calloc(name_idx + 1, sizeof(char));
    memcpy(dir, name, name_idx);
    dir[name_idx] = '\0';
    file = &name[name_idx];
//     kprintf("dir: %s, file: %s\n", dir, file);
    
    // Create the file
    id = open_path(dir, 0);
    free(dir);
    kprintf("Dir open: %p\n", id);
    if (!id) {
        return ENOENT;
    }
    
    err = kapi_urs_create(id, file, ucreate_node, 0, NULL);
    kprintf("Node create: %d\n", err);
    if (err) {
        return err;
    }
    
    kprintf("To close dir, id: %p\n", id);
    err = kapi_urs_close(id);
    kprintf("Dir close: %p (%d)\n", id, err);
    if (err) {
        return err;
    }
    
    // Actually touch the file
    id = open_path(name, 0);
    if (id) {
        kprintf("Open: %p\n", id);
        err = touch_file(id);
    } else {
        err = ENOENT;
    }
    
    return err;
}

static int do_touch(char *name)
{
    int err = EOK;
    unsigned long id = open_path(name, 0);
    
    if (id) {
        kprintf("Open: %p\n", id);
        err = touch_file(id);
    }
    
    else {
//         kprintf("To create new file\n");
        err = create_file(name);
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
    
    return err;
}
