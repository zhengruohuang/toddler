#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


int cd(int argc, char **argv)
{
    int err = EOK;
    
    if (argc < 2) {
        return EOK;
    }
    
    // Find out the new cwd
    char *join = NULL;
    char *new_dir = NULL;
    char *dir = argv[1];
    if (is_absolute_path(dir)) {
        join = strdup(dir);
    } else {
        char *cwd = get_cwd();
        join = join_path(cwd, dir);
        free(cwd);
    }
    
    // Normalize
    new_dir = normalize_path(join);
    free(join);
    
    // Try opening the dir
    err = change_cwd(new_dir);
    
    // Clean up
    free(new_dir);
    
    return err;
}
