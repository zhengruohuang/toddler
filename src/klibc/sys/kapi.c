#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"


msg_t *kapi_msg(int kapi_num)
{
    msg_t *msg = syscall_msg();
    
    msg->mailbox_id = IPC_MAILBOX_KERNEL;
    msg->opcode = IPC_OPCODE_KAPI;
    msg->func_num = kapi_num;
    
    return msg;
}

unsigned long kapi_return_value(msg_t *m)
{
    return msg_return_value(m);
}

unsigned long msg_return_value(msg_t *m)
{
    int index = m->param_count - 1;
    m->param_count--;
    if (index < 0) {
        return -1;
    }
    
    return m->params[index].value;
}

void *msg_return_buffer(msg_t *m, size_t *size)
{
    unsigned char *dest = NULL, *src = NULL;
    
    int index = m->param_count - 1;
    m->param_count--;
    if (index < 0) {
        return NULL;
    }
    
    src = (unsigned char *)m + m->params[index].offset;
    dest = (unsigned char *)malloc(sizeof(m->params[index].size));
    memcpy(dest, src, m->params[index].size);
    
    if (size) {
        *size = m->params[index].size;
    }
    
    return (void *)dest;
}

void msg_param_value(msg_t *m, unsigned long value)
{
    int index = m->param_count;
    
    m->params[index].type = MSG_PARAM_VALUE;
    m->params[index].value = value;
    
    m->param_count++;
}

void msg_param_buffer(msg_t *m, void *buf, size_t size)
{
    int index = m->param_count;
    unsigned char *src = NULL;
    unsigned char *dest = NULL;
    unsigned char temp = 0;
    int i;
    
    // Align the size
    int extra_msg_size = (int)size;
    if (extra_msg_size % (int)sizeof(unsigned long)) {
        extra_msg_size /= (int)sizeof(unsigned long);
        extra_msg_size++;
        extra_msg_size *= (int)sizeof(unsigned long);
    }
    
    // Set param
    m->params[index].type = MSG_PARAM_BUFFER;
    m->params[index].offset = m->msg_size;
    m->params[index].size = (int)size;
    
    // Copy the buffer content
    if (buf && size) {
        src = (unsigned char *)buf;
        dest = ((unsigned char *)m) + m->msg_size;
        
        for (i = 0; i < size; i++) {
            *dest++ = *src++;
        }
    }
    
    // Set size and count
    m->msg_size += extra_msg_size;
    m->param_count++;
}


/*
 * KAPI registration helper
 * systme and driver will call this to register KAPIs and handlers
 */
int kapi_reg(unsigned long kapi_num, msg_handler_t handler)
{
    syscall_reg_kapi_server(kapi_num);
    syscall_reg_msg_handler(kapi_num, handler);
    
    return 0;
}
