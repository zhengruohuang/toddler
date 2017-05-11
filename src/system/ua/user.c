#include "common/include/data.h"
#include "common/include/ua.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/assert.h"
#include "klibc/include/stdstruct.h"


struct ua_user {
    unsigned long user_id;
    char *name;
    
    unsigned long session_count;
};

struct ua_builtin_user_info {
    unsigned long user_id;
    const char *name;
};


static int user_salloc_id = -1;

static hash_t *user_id_table;
static hash_t *user_name_table;
static unsigned long cur_avail_user_id = UA_USER_ID_ALLOC_BASE;

static struct ua_builtin_user_info builtin_users[] = {
    { UA_USER_ID_ROOT, "root" },
    { UA_USER_ID_SYSTEM, "system" },
};


/*
 * Node Hash
 */
static unsigned int ua_user_hash_func(void *key, unsigned int size)
{
    char *str = (char *)key;
    unsigned int k = 0;
    
    do {
        k += (unsigned int)(*str);
    } while (*str++);
    
    return k % size;
}

static int ua_user_hash_cmp(void *cmp_key, void *node_key)
{
    char *cmp_ch = (char *)cmp_key;
    char *node_ch = (char *)node_key;
    
    return strcmp(cmp_ch, node_ch);
}


/*
 * User ID
 */
unsigned long alloc_user_id()
{
    cur_avail_user_id++;
    return cur_avail_user_id;
}


/*
 * Get user
 */
struct ua_user *get_user_by_id(unsigned long user_id)
{
}

struct ua_user *get_user_by_name(char *name)
{
}


/*
 * Add user
 */
int add_user(unsigned long user_id, const char *name)
{
    struct ua_user *user = (struct ua_user *)salloc(user_salloc_id);
    user->user_id = user_id;
    user->name = strdup(name);
    user->session_count = 0;
    
    int success1 = hash_insert(user_id_table, (void *)user_id, user);
    int success2 = hash_insert(user_name_table, (void *)user->name, user);
    assert(success1 && success2);
    
    return success1 && success1;
}

static void add_builtin_users()
{
    int builtin_user_count = sizeof(builtin_users) / sizeof(struct ua_builtin_user_info);
    int i;
    
    for (i = 0; i < builtin_user_count; i++) {
        int succeed = add_user(builtin_users[i].user_id, builtin_users[i].name);
        assert(succeed);
    }
}


/*
 * Init
 */
void init_ua_user()
{
    user_salloc_id = salloc_create(sizeof(struct ua_user), 0, NULL, NULL);
    user_id_table = hash_new(0, NULL, NULL);
    user_name_table = hash_new(0, ua_user_hash_func, ua_user_hash_cmp);
    
    add_builtin_users();
}

void init_ua()
{
    init_ua_user();
}
