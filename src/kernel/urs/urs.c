/*
 * Universal Resource System - URS
 */

#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/errno.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/lib.h"
#include "kernel/include/ds.h"
#include "kernel/include/kapi.h"
#include "kernel/include/urs.h"


static int super_salloc_id;
static int node_salloc_id;
static int open_salloc_id;

static hashtable_t *super_table;
static hashtable_t *open_table;


/*
 * Hash table
 */
static ulong urs_hash_func(ulong key, ulong size)
{
    char *str = (char *)key;
    unsigned int k = 0;
    
    do {
        k += (unsigned int)(*str);
    } while (*str++);
    
    return k % size;
}

static int urs_hash_cmp(ulong cmp_key, ulong node_key)
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
    super_table = hashtable_new(0, urs_hash_func, urs_hash_cmp);
    open_table = hashtable_new(0, urs_hash_func, urs_hash_cmp);
    
    super_salloc_id = salloc_create(sizeof(struct urs_super), 0, 0, NULL, NULL);
    node_salloc_id = salloc_create(sizeof(struct urs_node), 0, 0, NULL, NULL);
    open_salloc_id = salloc_create(sizeof(struct urs_open), 0, 0, NULL, NULL);
}


/*
 * Super
 */
#define DEFAULT_NAMESPACE           "vfs://"
#define DEFAULT_NAMESPACE_LENGTH    (sizeof(DEFAULT_NAMESPACE) - 1)

static struct urs_super *get_super_by_id(unsigned long id)
{
    return (struct urs_super *)id;
}

static char *normalize_path(char *path)
{
    char *copy = NULL;
    
    int len = strlen(path);
    if (!len) {
        return NULL;
    }
    
    if (len >= 3 && path[len - 1] == '/' && (path[len - 2] != '/' || path[len - 3] != ':')) {
        len--;
    }
    
    if (path[0] == '/') {
        len += DEFAULT_NAMESPACE_LENGTH;   // vfs://
    }
    
    copy = (char *)calloc(len + 1, sizeof(char));
    if (path[0] == '/') {
        memcpy(DEFAULT_NAMESPACE, copy, 6);
        memcpy(path, copy + DEFAULT_NAMESPACE_LENGTH, len - DEFAULT_NAMESPACE_LENGTH);
    } else {
        memcpy(path, copy, len);
    }
    copy[len] = '\0';
    
//     kprintf("len: %d\n", len);
//     kprintf(path);
//     kprintf("Path normalized: %s -> %s @ %p\n", path, copy, copy);
    
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
        kprintf("copy: %s\n", copy);
        if (hashtable_contains(super_table, (ulong)copy)) {
            found = 1;
            break;
        }
        
        do {
            cur_pos--;
        } while (cur_pos > 0 && copy[cur_pos] != '/');
        
        if (cur_pos && copy[cur_pos] == '/') {
            copy[cur_pos + 1] = '\0';
        } else {
            copy[cur_pos] = '\0';
        }
    } while (cur_pos > 0);
    
    if (!found) {
        free(copy);
        return NULL;
    }
    
    super = (struct urs_super *)hashtable_obtain(super_table, (ulong)copy);
    assert(super);
    super->ref_count++;
    hashtable_release(super_table, (ulong)copy, super);
    
    kprintf("Super matched: %s ~ %s @ %p\n", copy, super->path, super);
    
    return super;
}

static struct urs_super *obtain_super(char *path)
{
    struct urs_super *super = NULL;
    
    char *copy = normalize_path(path);
    if (!copy) {
        return NULL;
    }
    
    super = (struct urs_super *)hashtable_obtain(super_table, (ulong)copy);
    if (!super) {
        free(copy);
        return NULL;
    }
    
    super->ref_count++;
    hashtable_release(super_table, (ulong)copy, super);
    free(copy);
    
    return super;
}

static struct urs_super *release_super(struct urs_super *s)
{
    s->ref_count--;
}

unsigned long urs_register(char *path)
{
    int i;
    struct urs_super *super = NULL;
    
    char *copy = normalize_path(path);
    if (!copy) {
        return 0;
    }
    
    super = obtain_super(copy);
    if (super) {
        release_super(super);
        free(copy);
        return 0;
    }
    
    super = (struct urs_super *)salloc(super_salloc_id);
    super->id = (unsigned long)super;
    super->path = copy;
    super->ref_count = 1;
    
    for (i = 0; i < uop_count; i++) {
        super->ops[i].type = udisp_none;
    }
    
    hashtable_insert(super_table, (ulong)copy, super);
    return super->id;
}

int urs_unregister(char *path)
{
    return 0;
}

int urs_register_op(
    unsigned long id, enum urs_op_type op, void *func,
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
static no_opt struct thread_control_block *get_tcb()
{
    unsigned long addr = 0;
    
    __asm__ __volatile__
    (
        "xorl   %%esi, %%esi;"
        "movl   %%gs:(%%esi), %%edi;"
        : "=D" (addr)
        :
        : "%esi"
    );
    
    return (struct thread_control_block *)addr;
}

static msg_t *send_request_msg()
{
    struct thread_control_block *tcb = get_tcb();
    
    msg_t *m = (msg_t *)tcb->msg_send;
    struct process *p = (struct process *)tcb->proc_id;
    struct thread *t = (struct thread *)tcb->thread_id;
    
    send_kernel(m, p, t);
    
    return (msg_t *)tcb->msg_recv;
}

static msg_t *create_dispatch_msg(struct urs_super *super, enum urs_op_type op, unsigned long node_id)
{
    msg_t *msg = create_request_msg();
    
    msg->mailbox_id = super->ops[op].mbox_id;
    msg->opcode = super->ops[op].msg_opcode;
    msg->func_num = super->ops[op].msg_func_num;
    
    set_msg_param_value(msg, (unsigned long)op);
    set_msg_param_value(msg, super->id);
    set_msg_param_value(msg, node_id);
    
    return msg;
}

static int dispatch_lookup(struct urs_super *super, unsigned long node_id, char *next, int *is_link,
                           unsigned long *next_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_lookup;
    
    kprintf("To dispatch lookup @ %s\n", next);
    
    if (super->ops[op].type == udisp_none) {
        if (is_link) {
            *is_link = 0;
        }
        if (next_id) {
            *next_id = 0;
        }
        
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, next, is_link, next_id, buf, count, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        ulong len = 0;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_buf(s, next, (size_t)(strlen(next) + 1));
        set_msg_param_value(s, count);
        
        r = send_request_msg();
        if (is_link) {
            *is_link = (int)r->params[0].value;
        }
        if (next_id) {
            *next_id = r->params[1].value;
        }
        
        r = send_request_msg();
        if (buf) {
            u8 *src = (u8 *)((unsigned long)r + r->params[0].offset);
            u8 *dest = buf;
            ulong s = r->params[1].value;
            
            while (len < count && len < s) {
                dest[len] = src[len];
                len++;
            }
        }
        if (actual) {
            *actual = len;
        }
        
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_open(struct urs_super *super, unsigned long node_id)
{
    int result = 0;
    enum urs_op_type op = uop_open;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        s = create_dispatch_msg(super, op, node_id);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;
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
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        s = create_dispatch_msg(super, op, node_id);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;
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
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, buf, count, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        ulong len = 0;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_value(s, count);
        
        r = send_request_msg();
        if (buf) {
            u8 *src = (u8 *)((unsigned long)r + r->params[0].offset);
            u8 *dest = buf;
            ulong s = r->params[1].value;
            
            while (len < count && len < s) {
                dest[len] = src[len];
                len++;
            }
        }
        if (actual) {
            *actual = len;
        }
        
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_write(struct urs_super *super, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_write;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, buf, count, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_buf(s, buf, count);
        set_msg_param_value(s, count);
        
        r = send_request_msg();
        if (actual) {
            *actual = r->params[0].value;
        }
        
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_truncate(struct urs_super *super, unsigned long node_id)
{
    int result = 0;
    enum urs_op_type op = uop_write;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        s = create_dispatch_msg(super, op, node_id);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_seek_data(struct urs_super *super, unsigned long node_id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    int result = 0;
    enum urs_op_type op = uop_seek_data;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, offset, from, newpos);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_value(s, offset);
        set_msg_param_value(s, (unsigned long)from);
        
        r = send_request_msg();
        if (newpos) {
            *newpos = r->params[0].value;
        }
        
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_list(struct urs_super *super, unsigned long node_id, void *buf, unsigned long count, unsigned long *actual)
{
    int result = 0;
    enum urs_op_type op = uop_list;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, buf, count, actual);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        ulong len = 0;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_value(s, count);
        
        r = send_request_msg();
        if (buf) {
            u8 *src = (u8 *)((unsigned long)r + r->params[0].offset);
            u8 *dest = buf;
            ulong s = r->params[1].value;
            
            while (len < count && len < s) {
                dest[len] = src[len];
                len++;
            }
        }
        if (actual) {
            *actual = len;
        }
        
        result = (int)r->params[r->param_count - 1].value;
    }
    
    else {
        return -2;
    }
    
    return result;
}

static int dispatch_seek_list(struct urs_super *super, unsigned long node_id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    int result = 0;
    enum urs_op_type op = uop_seek_list;
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, offset, from, newpos);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_value(s, offset);
        set_msg_param_value(s, (unsigned long)from);
        
        r = send_request_msg();
        if (newpos) {
            *newpos = r->params[0].value;
        }
        
        result = (int)r->params[r->param_count - 1].value;
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
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, name);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_buf(s, name, strlen(name) + 1);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;
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
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        s = create_dispatch_msg(super, op, node_id);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;;
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
    
    if (super->ops[op].type == udisp_none) {
        return -1;
    }
    
    else if (super->ops[op].type == udisp_func) {
        result = super->ops[op].func(super->id, node_id, name);
    }
    
    else if (super->ops[op].type == udisp_msg) {
        msg_t *s, *r;
        
        s = create_dispatch_msg(super, op, node_id);
        set_msg_param_buf(s, name, strlen(name) + 1);
        
        r = send_request_msg();
        result = (int)r->params[r->param_count - 1].value;
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
    
    if (name && *name) {
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
        memcpy(&path[start], copy, end - start);
        copy[end - start] = '\0';
        *name = copy;
    }
    
    return end + 1;
}

static struct urs_node *get_next_node(struct urs_super *super, struct urs_node *cur, char *name,
                                      int *is_link, void *buf, unsigned long count, unsigned long *actual)
{
    int error = 0;
    unsigned long next_id = 0;
    struct urs_node *next;
    
//     kprintf("get next node\n");
    
    if (!cur) {
        if (!name) {
            error = dispatch_lookup(super, 0, "/", is_link,
                                    &next_id, buf, count, actual);
        } else {
            return NULL;
        }
    } else {
        error = dispatch_lookup(super, cur->dispatch_id, name, is_link,
                                &next_id, buf, count, actual);
        sfree(cur);
    }
    
    if (!next_id || error) {
        kprintf("Next node empty!\n");
        return NULL;
    }
    
    next = (struct urs_node *)salloc(node_salloc_id);
    next->dispatch_id = next_id;
    next->id = (unsigned long)next;
    next->ref_count = 1;
    next->super = super;
    
    return next;
}

static int is_absolute_path(char *path)
{
    size_t i;
    size_t len = strlen(path);
    if (!len) {
        return 0;
    }
    
    if (path[0] == '/') {
        return 1;
    }
    
    for (i = 0; i < len; i++) {
        if (
            path[i] == ':' && i < len - 2 &&
            path[i + 1] == '/' && path[i + 2] == '/'
        ) {
            return 1;
        }
    }
    
    return 0;
}

static struct urs_node *follow_link(struct urs_super *super, struct urs_node *cur_node, char *link_path, struct urs_super **real_super)
{
    char *name = 0;
    char *copy = normalize_path(link_path);
    int cur_pos = 0;
    
    if (is_absolute_path(copy)) {
        super = match_super(link_path);
        if (real_super) {
            *real_super = super;
        }
        
        if (!super) {
            free(copy);
            return NULL;
        }
        
        cur_pos = strlen(super->path);
    } else {
        cur_pos = get_next_name(copy, cur_pos, &name);
    }
    
    do {
        int is_link;
        u8 buf[128];
        unsigned long link_len = 0;
        
        cur_node = get_next_node(super, cur_node, name, &is_link, buf, sizeof(buf), &link_len);
        if (is_link) {
            cur_node = follow_link(super, cur_node, (char *)buf, real_super);
        }
        
        cur_pos = get_next_name(copy, cur_pos, &name);
    } while (cur_pos && cur_node);
    
    free(copy);
    if (name) {
        free(name);
    }
    
    return cur_node;
}

static struct urs_node *resolve_path(char *path, struct urs_super **real_super)
{
    char *name = NULL;
    char *copy = normalize_path(path);
    int cur_pos = 0;
    struct urs_node *cur_node = NULL;
    
//     kprintf("to resolve: %s\n", path);
    
    // Find out super
    struct urs_super *super = match_super(copy);
    if (real_super) {
        *real_super = super;
    }
    if (!super) {
        free(copy);
        return NULL;
    }
    
    // Look up the path
    cur_pos = strlen(super->path);
    
    do {
        int is_link;
        u8 buf[128];
        unsigned long link_len = 0;
        
        cur_node = get_next_node(super, cur_node, name, &is_link, buf, sizeof(buf), &link_len);
        if (is_link) {
            cur_node = follow_link(super, cur_node, (char *)buf, real_super);
        }
        
        cur_pos = get_next_name(copy, cur_pos, &name);
    } while (cur_pos && cur_node);
    
    free(copy);
    if (name) {
        free(name);
    }
    
    return cur_node;
}

unsigned long urs_open_node(char *path, unsigned int mode, unsigned long process_id)
{
    struct urs_open *o = NULL;
    struct urs_super *super = NULL;
    
    // Resolve path
    struct urs_node *node = resolve_path(path, &super);
    if (!node) {
        return 0;
    }
    
    // Open the node
    if (dispatch_open(super, node->dispatch_id)) {
        return 0;
    }
    
    o = (struct urs_open *)salloc(open_salloc_id);
    o->id = (unsigned long)o;
    o->path = path;
    o->node = node;
    o->ref_count = 1;
    
    o->data_pos = 0;
    o->data_size = 0;
    o->list_pos = 0;
    o->list_size = 0;
    
    hashtable_insert(open_table, (ulong)path, o);
    return o->id;
}

int urs_close_node(unsigned long id, unsigned long process_id)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (!o) {
        return EBADF;
    }
    
    o->ref_count--;
    if (!o->ref_count) {
        error = dispatch_release(o->node->super, o->node->dispatch_id);
        if (!error) {
            hashtable_remove(open_table, (ulong)o->path);
            release_super(o->node->super);
            sfree(o->node);
//             free(o->path);
            sfree(o);
        }
    }
    
    return error;
}

int urs_read_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual)
{
    struct urs_open *o = get_open_by_id(id);
    int error = EOK;
    
    if (o) {
        error = dispatch_read(o->node->super, o->node->dispatch_id, buf, count, actual);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_write_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual)
{
    struct urs_open *o = get_open_by_id(id);
    int error = EOK;
    
    if (o) {
        error = dispatch_write(o->node->super, o->node->dispatch_id, buf, count, actual);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_truncate_node(unsigned long id)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_truncate(o->node->super, o->node->dispatch_id);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_seek_data(unsigned long id, unsigned long offset, enum urs_seek_from from, unsigned long *newpos)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_seek_data(o->node->super, o->node->dispatch_id, offset, from, newpos);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_list_node(unsigned long id, void *buf, unsigned long count, unsigned long *actual)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_list(o->node->super, o->node->dispatch_id, buf, count, actual);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_seek_list(long unsigned int id, long long unsigned int offset, enum urs_seek_from from, unsigned long *newpos)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_seek_list(o->node->super, o->node->dispatch_id, offset, from, newpos);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_create_node(unsigned long id, char *name)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_create(o->node->super, o->node->dispatch_id, name);
    } else {
        error = EBADF;
    }
    
    return error;
}

int urs_remove_node(unsigned long id)
{
    struct urs_open *o = get_open_by_id(id);
    int error = EOK;
    
    if (o->ref_count > 1) {
        return -1;
    }
    
    error = dispatch_remove(o->node->super, o->node->dispatch_id);
    if (error) {
        return error;
    }
    
    hashtable_remove(open_table, (ulong)o->path);
    release_super(o->node->super);
    sfree(o->node);
//     free(o->path);
    sfree(o);
    
    return 0;
}

int urs_rename_node(unsigned long id, char *name)
{
    int error = EOK;
    struct urs_open *o = get_open_by_id(id);
    
    if (o) {
        error = dispatch_rename(o->node->super, o->node->dispatch_id, name);
    } else {
        error = EBADF;
    }
    
    return error;
}
