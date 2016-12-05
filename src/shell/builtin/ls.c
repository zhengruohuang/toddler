#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static int check_flag(int argc, char **argv, char flag)
{
    int i, j;
    
    for (i = 1; i < argc; i++) {
        char *v = argv[i];
        
        if (!strlen(v) || v[0] != '-') {
            continue;
        }
        
        for (j = 0; j < strlen(v); j++) {
            if (v[j] == flag) {
                return 1;
            }
        }
    }
    
    return 0;
}

static void print_detailed_entry(char *path, char *name)
{
    kprintf("%s\n", name);
}

static int do_ls(char *path, int flag_list)
{
    int err = EOK;
    char buf[64];
    unsigned long id = open_path(path, 0);
//     kprintf("Open: %p\n", id);
    
    if (flag_list) {
        struct urs_stat stat;
        err = kapi_urs_stat(id, &stat);
//         assert(err = EOK);
        kprintf("Total entries: %lu\n", stat.sub_count);
    }
    
    int last = 0;
    do {
        last = kapi_urs_list(id, buf, sizeof(buf));
        if (!last) {
            if (flag_list) {
                print_detailed_entry(path, buf);
            } else {
                kprintf("%s ", buf);
            }
        }
    } while (!last);
    
    if (!flag_list) {
        kprintf("\n");
    }
    
    err = kapi_urs_close(id);
    return err;
}

int ls(int argc, char **argv)
{
    int err = EOK;
    int flag_list = check_flag(argc, argv, 'l');
    
    int i;
    int target_count = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            target_count++;
        }
    }
    
    if (target_count) {
        int i;
        
        for (i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                continue;
            }
            
            if (target_count > 1) {
                kprintf("%s:\n", argv[i]);
            }
            
            err = do_ls(argv[i], flag_list);
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
        err = do_ls(path, flag_list);
        free(path);
    }
    
    return err;
}
