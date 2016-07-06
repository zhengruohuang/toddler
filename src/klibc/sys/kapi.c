#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/sys.h"


/*
 * Helper functions
 */
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
    return m->params[0].value;
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
    src = (unsigned char *)buf;
    dest = ((unsigned char *)m) + m->msg_size;
    
    for (i = 0; i < size; i++) {
        *dest++ = *src++;
    }
    
    // Set size and count
    m->msg_size += extra_msg_size;
    m->param_count++;
}
