#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


/*
 * Path manipulation
 */
int is_valid_path(const char *path)
{
    return 1;
}

int is_absolute_path(const char *path)
{
    int i;
    int len = strlen(path);
    
    if (path && path[0] == '/') {
        return 1;
    }
    
    for (i = 0; i < len; i++) {
        if (path[i] == ':' && path[i + 1] == '/' && path[i + 2] == '/') {
            return 1;
        }
    }
    
    return 0;
}

int is_relative_path(const char *path)
{
    return !is_absolute_path(path);
}

char *join_path(const char *a, const char *b)
{
    int len_a = strlen(a);
    int len_b = strlen(b);
    int len = len_a + len_b;
    int idx = 0;
    char *p = NULL;
    
    // Find out final size of the new path
    if (b[0] == '/' && a[len_a - 1] == '/') {
        len--;
    } else if (b[0] != '/' && a[len_a - 1] != '/') {
        len++;
    }
    
    // Create the new path
    p = calloc(len + 1, sizeof(char));
    if (!p) {
        return NULL;
    }
    
    // Copy a
    memcpy(p, a, len_a);
    idx = len_a;
    if (a[len_a - 1] != '/') {
        p[idx] = '/';
        idx++;
    }
    
    // Copy b, '/' in b should be dropped
    if (b[0] == '/') {
        memcpy(&p[idx], &b[1], len_b - 1);
    } else {
        memcpy(&p[idx], b, len_b);
    }
    
    // Finalize
    p[len] = '\0';
    kprintf("Path joined: %s + %s -> %s, len: %d\n", a, b, p, len);
    
    return p;
}

char *normalize_path(const char *path)
{
    int ori_idx = 0;
    int dup_idx = 0;
    int dup_search = 0;
    int ori_len = strlen(path);
    char *norm = calloc(ori_len + 1, sizeof(char));
    
    dlist_t type_stack;
    dlist_t dup_stack;
    dlist_create(&dup_stack);
    dlist_create(&type_stack);
    
    do {
        // Extract the next part of ori path
        int is_namespace = 0;
        int ori_search = ori_idx;
        for (ori_search = ori_idx; ori_search < ori_len; ori_search++) {
            if (path[ori_search] == ':' && path[ori_search + 1] == '/' && path[ori_search + 2] == '/') {
                ori_search += 3;
                is_namespace = 1;
                break;
            } else if (path[ori_search] == '/') {
                ori_search += 1;
                break;
            }
        }
        
        // Go to the previous dir
        if (ori_search - ori_idx == 2 && path[ori_idx] == '.' && path[ori_idx + 1] == '.') {
            int type = (int)(unsigned long)dlist_pop_back(&type_stack);
            if (type) {
                dlist_push_back(&type_stack, (void *)(unsigned long)1);
            } else {
                dup_idx = (int)(unsigned long)dlist_pop_back(&dup_stack);
                kprintf("dup idx popped: %d\n", dup_idx);
            }
        }
        
        // If not staying in the same dir, then this is a regular dir
        else if (
            ori_search - ori_idx != 0 &&
            (ori_search - ori_idx != 1 || path[ori_idx] != '.')
        ) {
            int cpy_len = ori_search - ori_idx;
            memcpy(&norm[dup_idx], &path[ori_idx], cpy_len);
            
            kprintf("copy len: %d\n", cpy_len);
            
            dlist_push_back(&dup_stack, (void *)(unsigned long)dup_idx);
            dlist_push_back(&type_stack, (void *)(unsigned long)is_namespace);
            dup_idx += cpy_len;
        }
        
        kprintf("ori_search: %d, ori_idx: %d\n", ori_search, ori_idx);
        
        // Move to next part
        ori_idx = ori_search;
    } while (ori_idx < ori_len);
    
    // Finalize the string
    norm[dup_idx] = '\0';
    if (dup_idx && norm[dup_idx - 1] == '/') {
        if (dup_idx < 2 || norm[dup_idx - 2] != '/') {
            norm[dup_idx - 1] = '\0';
        }
    }
    
    // Clean up the stack
    do {
        dup_idx = (int)(unsigned long)dlist_pop_front(&dup_stack);
        dlist_pop_front(&type_stack);
    } while (dup_idx);
    kprintf("Path normalized: %s\n", norm);
    
    return norm;
}


/*
 * Working directory
 */
#define STARTUP_WORK_DIR    "coreimg://"

static char *cwd = NULL;
static unsigned long cwd_id = 0;

void init_cwd()
{
    cwd = strdup(STARTUP_WORK_DIR);
    cwd_id = kapi_urs_open(cwd, 0);
    assert(cwd_id);
}

char *get_cwd()
{
    return strdup(cwd);
}

int change_cwd(char *path)
{
    unsigned long id = kapi_urs_open(path, 0);
    if (!id) {
        return ENOENT;
    }
    
    free(cwd);
    cwd = strdup(path);
    
    kapi_urs_close(id);
    cwd_id = id;
    
    return EOK;
}


/*
 * Path operation helpers
 */
unsigned long open_path(char *name, unsigned int flags)
{
    unsigned long id = 0;
    
    // Find out the new cwd
    char *new_dir = NULL;
    if (is_absolute_path(name)) {
        new_dir = strdup(name);
    } else {
        char *cwd = get_cwd();
        new_dir = join_path(cwd, name);
        free(cwd);
    }
    
    // Try opening the dir
    id = kapi_urs_open(new_dir, flags);
    
    // Clean up
    free(new_dir);
    
    return id;
}
