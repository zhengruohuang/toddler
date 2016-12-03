#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


static char *find_norm_name(char *name)
{
    char *join = NULL;
    char *norm = NULL;
    
    // Join
    if (is_absolute_path(name)) {
        join = strdup(name);
    } else {
        char *cwd = get_cwd();
        join = join_path(cwd, name);
        free(cwd);
    }
    
    // Normalize
    norm = normalize_path(join);
    free(join);
    
    return norm;
}

static int is_same_dir(const char *a, const char *b)
{
}

static char *extract_dir(const char *full)
{
}

static char *extract_name(const char *full)
{
}

int mv(int argc, char **argv)
{
    int err = EOK;
    
    if (argc < 3) {
        return EOK;
    }
    
    // Find out the full name
    char *norm_old = find_norm_name(argv[1]);
    char *norm_new = find_norm_name(argv[2]);
    
    // If they are from the same dir, simply call urs rename
    if (is_same_dir(norm_old, norm_new)) {
        // Open the old file
        unsigned long id = open_path(norm_old, 0);
        if (!id) {
            err = ENOENT;
            goto cleanup;
        }
        
        // Rename it
        char *new_name = extract_name(norm_new);
        err = kapi_urs_rename(id, new_name);
        free(new_name);
    }
    
    // Otherwise create a new link, then unlink the old one
    else {
        // Open the new dir
        char *new_dir = extract_dir(norm_new);
        unsigned long new_dir_id = open_path(new_dir, 0);
        if (!new_dir_id) {
            err = ENOENT;
            goto cleanup;
        }
        
        // Create a link inside of the new dir
        char *new_name = extract_name(norm_new);
        err = kapi_urs_create(new_dir_id, new_name, ucreate_hard_link, 0, norm_old);
        free(new_name);
        if (err) {
            goto cleanup;
        }
        
        // Open the old file
        unsigned long old_id = open_path(norm_old, 0);
        if (!old_id) {
            err = ENOENT;
            goto cleanup;
        }
        
        // Unlink it
        err = kapi_urs_remove(old_id, 0);
    }
    
    // Clean up
cleanup:
    free(norm_old);
    free(norm_new);
    
    return err;
}
