#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/urs.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"


unsigned long kapi_urs_reg_super(char *path, char *name, unsigned int flags, struct urs_reg_ops *ops)
{
    msg_t *s = kapi_msg(KAPI_URS_REG_SUPER);
    msg_t *r;
    unsigned long result = 0;
    
    msg_param_buffer(s, path, (size_t)(strlen(path) + 1));
    msg_param_buffer(s, name, (size_t)(strlen(name) + 1));
    msg_param_value(s, (unsigned long)flags);
    msg_param_buffer(s, ops, (size_t)(sizeof(struct urs_reg_ops)));
    
    r = syscall_request();
    result = kapi_return_value(r);
    
    return result;
}

// int kapi_urs_reg_op(unsigned long super_id, enum urs_op_type op, unsigned long msg_opcode, unsigned long msg_func_num)
// {
//     msg_t *s = kapi_msg(KAPI_URS_REG_OP);
//     msg_t *r;
//     unsigned long result = 0;
//     
//     msg_param_value(s, super_id);
//     msg_param_value(s, (unsigned long)op);
// //     msg_param_value(s, mbox_id);
//     msg_param_value(s, msg_opcode);
//     msg_param_value(s, msg_func_num);
//     
//     r = syscall_request();
//     result = (int)kapi_return_value(r);
//     
//     return result;
// }

unsigned long kapi_urs_open(char *name, unsigned int flags)
{
    msg_t *s = kapi_msg(KAPI_URS_OPEN);
    msg_t *r;
    unsigned long result = 0;
    
    msg_param_buffer(s, name, (size_t)(strlen(name) + 1));
    msg_param_value(s, (unsigned long)flags);
    
    r = syscall_request();
    result = kapi_return_value(r);
    
    return result;
}

int kapi_urs_close(unsigned long fd)
{
    msg_t *s = kapi_msg(KAPI_URS_CLOSE);
    msg_t *r;
    int result = -1;
    
    msg_param_value(s, fd);
    r = syscall_request();
    result = (int)kapi_return_value(r);
    
    return result;
}

size_t kapi_urs_read(unsigned long fd, void *buf, size_t count)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_URS_READ);
    msg_t *r = NULL;
//     int result = -1;
    
    // Setup the params
    msg_param_value(s, fd);
    msg_param_value(s, (unsigned long)count);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    void *data = (void *)((unsigned long)r + r->params[0].offset);
    size_t len = (size_t)r->params[1].value;
    if (len && buf && count) {
        memcpy(buf, data, len);
    } else {
        len = 0;
    }
    
    // Setup the result
//     result = (int)kapi_return_value(r);
    
    return len;
}

size_t kapi_urs_write(unsigned long fd, void *buf, size_t count)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_URS_WRITE);
    msg_t *r = NULL;
    int result = -1;
    
    // Setup the params
    msg_param_value(s, fd);
    msg_param_buffer(s, buf, count);
    msg_param_value(s, (unsigned long)count);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    
    // Setup the result
    result = (int)kapi_return_value(r);
    
    return result;
}

size_t kapi_urs_list(unsigned long fd, void *buf, size_t count)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_URS_LIST);
    msg_t *r = NULL;
    int result = -1;
    
    // Setup the params
    msg_param_value(s, fd);
    msg_param_value(s, (unsigned long)count);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    void *data = (void *)((unsigned long)r + r->params[0].offset);
    size_t len = (size_t)r->params[1].value;
    if (buf && count) {
        memcpy(buf, data, len);
    } else {
        len = 0;
    }
    
    // Setup the result
    result = (int)kapi_return_value(r);
    
    return result;
}

int kapi_urs_create(unsigned long fd, char *name, enum urs_create_type type, unsigned int flags, char *target)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_URS_CREATE);
    msg_t *r = NULL;
    int result = -1;
    
    // Setup the params
    msg_param_value(s, fd);
    msg_param_buffer(s, name, strlen(name) + 1);
    msg_param_value(s, (unsigned long)type);
    msg_param_value(s, (unsigned long)flags);
    msg_param_buffer(s, target, target ? strlen(name) + 1 : 0);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    
    // Setup the result
    result = (int)kapi_return_value(r);
    
    return result;
}

int kapi_urs_remove(unsigned long fd, int erase)
{
    // Setup the msg
    msg_t *s = kapi_msg(KAPI_URS_REMOVE);
    msg_t *r = NULL;
    int result = -1;
    
    // Setup the params
    msg_param_value(s, fd);
    msg_param_value(s, (unsigned long)erase);
    
    // Issue the KAPI and obtain the result
    r = syscall_request();
    
    // Setup the result
    result = (int)kapi_return_value(r);
    
    return result;
}

//  int lseek(int file, int ptr, int dir);

//  int fstat(int file, struct stat *st);
//  int stat(const char *file, struct stat *st);

//  int link(char *old, char *new);
//  int unlink(char *name);

//  int isatty(int file);
