/*
 * Universal Resource System - URS
 */

#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "system/include/urs.h"


static int super_salloc_id;
static int node_salloc_id;
static int open_salloc_id;

static hash_t *super_table;
static hash_t *open_table;


/*
 * Hash table
 */
static unsigned int urs_hash_func(void *key, unsigned int size)
{
    char *str = (char *)key;
    unsigned int k = 0;
    
    do {
        k += (unsigned int)(*str);
    } while (*str++);
    
    return k % size;
}

static int urs_hash_cmp(void *cmp_key, void *node_key)
{
    char *cmp_ch = (char *)cmp_key;
    char *node_ch = (char *)node_key;
    
    return strcmp(cmp_ch, node_ch);
}


/*
 * Init URS
 */
void init_urs()
{
    super_table = hash_new(0, urs_hash_func, urs_hash_cmp);
    open_table = hash_new(0, urs_hash_func, urs_hash_cmp);
    
    super_salloc_id = salloc_create(sizeof(struct urs_super), 0, NULL, NULL);
    node_salloc_id = salloc_create(sizeof(struct urs_node), 0, NULL, NULL);
    open_salloc_id = salloc_create(sizeof(struct urs_open), 0, NULL, NULL);
}


/*
 * Super
 */
static struct urs_super *get_super_by_id(unsigned long id)
{
    return (struct urs_super *)id;
}

static char *normalize_path(char *path)
{
    char *copy = NULL;
    
    int len = strlen(path);
    if (len == 1 && path[0] == '/') {
        copy = strdup("vfs://");
    } else if (len <= 3) {
        return NULL;
    }
    
    if (path[len - 1] == '/') {
        len--;
    }
    
    copy = (char *)calloc(len + 1, sizeof(char));
    memcpy(copy, path, len);
    copy[len] = '\0';
    
    return copy;
}

static struct urs_super *match_super(char *path)
{
    int found = 0;
    struct urs_super *super = NULL;
    
    int cur_pos = 0;
    char *copy = normalize_path(path);
    if (!copy) {
        return NULL;
    }
    cur_pos = strlen(copy) - 1;
    
    do {
        if (hash_contains(super_table, copy)) {
            found = 1;
            break;
        }
        
        do {
            cur_pos--;
        } while (cur_pos > 0 && copy[cur_pos] != '/');
        
        if (!cur_pos) {
            break;
        }
        copy[cur_pos] = '\0';
    } while (cur_pos > 0 && copy[cur_pos] != ':');
    
    if (!found) {
        free(copy);
        return NULL;
    }
    
    super = (struct urs_super *)hash_obtain(super_table, copy);
    assert(super);
    super->ref_count++;
    hash_release(super_table, copy, super);
    
    return super;
}

static struct urs_super *obtain_super(char *path)
{
    struct urs_super *super = NULL;
    
    char *copy = normalize_path(path);
    if (!copy) {
        return NULL;
    }
    
    super = (struct urs_super *)hash_obtain(super_table, copy);
    if (!super) {
        free(copy);
        return NULL;
    }
    
    super->ref_count++;
    hash_release(super_table, copy, super);
    free(copy);
    
    return super;
}

static struct urs_super *release_super(struct urs_super *s)
{
    s->ref_count--;
}

struct urs_super *register_super(char *path)
{
    int i;
    struct urs_super *super = NULL;
    
    char *copy = normalize_path(path);
    if (!copy) {
        return NULL;
    }
    
    super = obtain_super(copy);
    if (super) {
        release_super(super);
        free(copy);
        return NULL;
    }
    
    super = (struct urs_super *)salloc(super_salloc_id);
    super->id = (unsigned long)super;
    super->path = copy;
    super->ref_count = 1;
    
    for (i = 0; i < uop_count; i++) {
        super->ops[i].type = udisp_none;
    }
    
    hash_insert(super_table, copy, super);
    return super;
}

int register_super_op(
    unsigned long id, enum urs_op_type op, void *func, char *link,
    unsigned long mbox_id, unsigned long msg_opcode, unsigned long msg_func_num)
{
    struct urs_super *super = get_super_by_id(id);
    if (!super) {
        return -1;
    }
    
    if (func) {
        super->ops[op].type = udisp_func;
        super->ops[op].func = func;
    }
    
    else if (link) {
        super->ops[op].type = udisp_link;
        super->ops[op].link = strdup(link);
    }
    
    else {
        super->ops[op].type = udisp_msg;
        super->ops[op].mbox_id = mbox_id;
        super->ops[op].msg_opcode = msg_opcode;
        super->ops[op].msg_func_num = msg_func_num;
    }
    
    
    return 0;
}


/*
 * Dispatch
 */
static msg_t *create_dispatch_msg(struct urs_super *super, enum urs_op_type op, unsigned long dispatch_id)
{
    msg_t *msg = syscall_msg();
    
    msg->mailbox_id = super->ops[op].mbox_id;
    msg->opcode = super->ops[op].msg_opcode;
    msg->func_num = super->ops[op].msg_func_num;
    
    msg_param_value(msg, (unsigned long)op);
    msg_param_value(msg, super->id);
    msg_param_value(msg, dispatch_id);
    
    return msg;
}

static int dispatch_lookup(struct urs_super *super, unsigned long node_id, char *next, unsigned long *next_id)
{
    int result = 0;
    enum urs_op_type op = uop_lookup;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, next, next_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        msg_param_buffer(s, next, (size_t)(strlen(next) + 1));
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_release(struct urs_super *super, unsigned long node_id)
{
    int result = 0;
    enum urs_op_type op = uop_release;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_read(struct urs_super *super, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_read;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, buf, count, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        msg_param_value(s, count);
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_write(struct urs_super *super, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_read;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, count, buf, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        msg_param_value(s, count);
        msg_param_buffer(s, buf, count);
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
        
        if (actual) {
            *actual = count;
        }
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_list(struct urs_super *super, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_read;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, count, buf, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
        
        if (actual) {
            *actual = count;
        }
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_create(struct urs_super *super, unsigned long node_id, char *name)
{
    int result = 0;
    enum urs_op_type op = uop_create;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, name);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        msg_param_buffer(s, name, strlen(name));
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_remove(struct urs_super *super, unsigned long node_id)
{
    int result = 0;
    enum urs_op_type op = uop_remove;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_rename(struct urs_super *super, unsigned long node_id, char *name)
{
    int result = 0;
    enum urs_op_type op = uop_rename;
    msg_t *s, *r;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, name);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        s = create_dispatch_msg(super, op, node_id);
        msg_param_buffer(s, name, strlen(name));
        
        r = syscall_request();
        result = (int)kapi_return_value(r);
    }
    
    else {
        return -2;
    }
    
    return result;
}


/*
 * Node operations
 */
static struct urs_open *get_open_by_id(unsigned long id)
{
    return (struct urs_open *)id;
}

static int get_next_name(char *path, int start, char **name)
{
    char *copy = NULL;
    int end = 0;
    int len = 0;
    
    if (*name) {
        free(*name);
        *name = NULL;
    }
    
    len = strlen(path);
    if (start >= len) {
        return 0;
    }
    
    if (path[start] == '/') {
        start++;
        if (start >= len) {
            return 0;
        }
    }
    
    end = start;
    while (path[end] != '/' && end < len) {
        end++;
    }
    
    if (name) {
        copy = (char *)calloc(end - start + 1, sizeof(char));
        memcpy(copy, &path[start], end - start);
        copy[end - start] = '\0';
        *name = copy;
    }
    
    return end + 1;
}

static struct urs_node *get_next_node(struct urs_super *super, struct urs_node *cur, char *name)
{
    int error = 0;
    unsigned long next_id = 0;
    struct urs_node *next;
    
    if (!cur) {
        if (!name) {
            error = dispatch_lookup(super, 0, "/", &next_id);
        } else {
            return NULL;
        }
    } else {
        error = dispatch_lookup(super, cur->dispatch_id, name, &next_id);
        error |= dispatch_release(super, cur->dispatch_id);
        sfree(cur);
    }
    
    if (!next_id || error) {
        return NULL;
    }
    
    next = (struct urs_node *)salloc(node_salloc_id);
    next->dispatch_id = next_id;
    next->id = (unsigned long)next;
    next->ref_count = 1;
    next->super = super;
    
    return next;
}

struct urs_open *open_node(char *path, unsigned int mode, unsigned long process_id)
{
    char *name = NULL;
    int cur_pos = 0;
    struct urs_node *cur_node = NULL;
    struct urs_open *o = NULL;
    
    char *copy = NULL;
    struct urs_super *super = match_super(path);
    if (!super) {
        return NULL;
    }
    
    copy = normalize_path(path);
    cur_pos = strlen(super->path);
    
    do {
        cur_node = get_next_node(super, cur_node, name);
        cur_pos = get_next_name(copy, cur_pos, &name);
    } while (cur_pos && cur_node);
    
    if (name) {
        free(name);
    }
    
    if (!cur_node) {
        return NULL;
    }
    
    o = (struct urs_open *)salloc(open_salloc_id);
    o->id = (unsigned long)o;
    o->path = copy;
    o->node = cur_node;
    o->ref_count = 1;
    
    o->cur_pos = 0;
    o->size = 0;
    
    hash_insert(open_table, copy, o);
    return o;
}

int close_node(unsigned long id, unsigned long process_id)
{
    int error = -0;
    struct urs_open *o = get_open_by_id(id);
    
    o->ref_count--;
    if (!o->ref_count) {
        error = dispatch_release(o->node->super, o->node->dispatch_id);
        if (!error) {
            hash_remove(open_table, o->path);
            release_super(o->node->super);
            sfree(o->node);
            free(o->path);
            sfree(o);
        }
    }
    
    return error;
}

unsigned long read_node(unsigned long id, void *buf, unsigned long count)
{
    struct urs_open *o = get_open_by_id(id);
    unsigned long actual = 0;
    int error = dispatch_read(o->node->super, o->node->id, buf, count, &actual);
    return error ? 0 : actual;
}

unsigned long write_node(unsigned long id, void *buf, unsigned long count)
{
    struct urs_open *o = get_open_by_id(id);
    unsigned long actual = 0;
    int error = dispatch_read(o->node->super, o->node->id, buf, count, &actual);
    return error ? 0 : actual;
}

int list_node(unsigned long id, void *buf, unsigned long count)
{
    struct urs_open *o = get_open_by_id(id);
    unsigned long actual = 0;
    int error = dispatch_list(o->node->super, o->node->id, buf, count, &actual);
    return error;
}

int urs_create_node(unsigned long id, char *name)
{
    struct urs_open *o = get_open_by_id(id);
    int error = dispatch_create(o->node->super, o->node->id, name);
    return error;
}

int remove_node(unsigned long id)
{
    struct urs_open *o = get_open_by_id(id);
    int error = 0;
    
    if (o->ref_count > 1) {
        return -1;
    }
    
    error = dispatch_remove(o->node->super, o->node->id);
    if (error) {
        return error;
    }
    
    hash_remove(open_table, o->path);
    release_super(o->node->super);
    sfree(o->node);
    free(o->path);
    sfree(o);
    
    return 0;
}

int rename_node(unsigned long id, char *name)
{
    struct urs_open *o = get_open_by_id(id);
    int error = dispatch_rename(o->node->super, o->node->id, name);
    return error;
}
