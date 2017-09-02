/*
 * KAPI Handling - URS
 */

#include "common/include/syscall.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/sys.h"
#include "system/include/urs.h"
#include "system/include/kapi.h"


/*
 * Super operations
 */
asmlinkage void urs_reg_super_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    char *path = (char *)((ulong)s + s->params[0].offset);
    char *name = (char *)((ulong)s + s->params[1].offset);
    int flags = (int)s->params[2].value;
    struct urs_reg_ops *ops = (struct urs_reg_ops *)((ulong)s + s->params[3].offset);
    ops->mbox_id = s->sender_proc_id;   // FIXME: should be proc_id
    
    int result = (int)urs_register(path, name, flags, ops);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

// asmlinkage void urs_reg_op_handler(msg_t *s)
// {
//     struct thread *t = arg->sender_thread;
//     msg_t *s = arg->msg;
//     msg_t *r = create_response_msg(t);
//     
//     ulong super_id = s->params[0].value;
//     enum urs_op_type op = (enum urs_op_type)s->params[1].value;
// //     ulong mbox_id = s->params[2].value;
//     ulong msg_opcode = s->params[2].value;
//     ulong msg_func_num = s->params[3].value;
//     
//     int result = (int)urs_register_op(super_id, op, NULL, arg->sender_thread->proc_id, msg_opcode, msg_func_num);
//     msg_param_value(r, (ulong)result);
//     
//     run_thread(t);
//     
//     // Clean up
//     terminate_thread_self(arg->handler_thread);
//     sfree(arg);
//     
//     // Wait for this thread to be terminated
//     ksys_unreachable();
// }


/*
 * Node operations
 */
asmlinkage void urs_open_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    char *name = (char *)((ulong)s + s->params[0].offset);
    unsigned int flags = (unsigned int)s->params[1].value;
//     unsigned long open_id = 0;
    
    unsigned long result = urs_open_node(name, flags, s->sender_proc_id);   // FIXME: should be proc_id
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_close_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    int result = (int)urs_close_node(open_id);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_read_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    u8 read_buf[128];
    ulong open_id = s->params[0].value;
    int buf_size = (int)s->params[1].value;
    if (buf_size > sizeof(read_buf)) {
        buf_size = sizeof(read_buf);
    }
    ulong len = 0;
    
    int result = (int)urs_read_node(open_id, read_buf, buf_size, &len);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_buffer(r, read_buf, len);
    msg_param_value(r, len);
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_write_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    void *write_buf = (void *)((ulong)s + s->params[1].offset);
    int buf_size = (int)s->params[2].value;
    ulong len = 0;
    
    int result = (int)urs_write_node(open_id, write_buf, buf_size, &len);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, len);
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_list_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    u8 name_buf[128];
    ulong open_id = s->params[0].value;
    int buf_size = (int)s->params[1].value;
    if (buf_size > sizeof(name_buf)) {
        buf_size = sizeof(name_buf);
    }
    ulong len = 0;
    
    int result = (int)urs_list_node(open_id, name_buf, buf_size, &len);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_buffer(r, name_buf, len);
    msg_param_value(r, len);
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_create_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    char *name = (char *)((ulong)s + s->params[1].offset);
    enum urs_create_type type = s->params[2].value;
    unsigned int flags = (unsigned int)s->params[3].value;
    char *target = s->params[4].offset ? (char *)((ulong)s + s->params[4].offset) : NULL;
    
    int result = (int)urs_create_node(open_id, name, type, flags, target);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_remove_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    int erase = (int)s->params[1].value;
    
    int result = (int)urs_remove_node(open_id, erase);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_rename_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    char *name = (char *)((ulong)s + s->params[1].offset);
    
    int result = (int)urs_rename_node(open_id, name);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}

asmlinkage void urs_stat_handler(msg_t *s)
{
    unsigned long reply_mbox_id = s->mailbox_id;
    ulong open_id = s->params[0].value;
    struct urs_stat stat;
    
    int result = (int)urs_stat_node(open_id, &stat);
    
    msg_t *r = syscall_msg();
    r->mailbox_id = reply_mbox_id;
    msg_param_buffer(r, &stat, sizeof(struct urs_stat));
    msg_param_value(r, (ulong)result);
    
    syscall_respond();
    sys_unreahable();
}
