#include "common/include/data.h"
#include "common/include/syscall.h"
#include "klibc/include/string.h"
#include "klibc/include/assert.h"
#include "klibc/include/sys.h"


unsigned long kapi_stdin_read(unsigned long console_id, void *buf, unsigned long count)
{
    msg_t *s = kapi_msg(KAPI_STDIN_READ);
    msg_param_value(s, console_id);
    msg_param_value(s, count);
    
    msg_t *r = syscall_request();
    char *data = (char *)((unsigned long)r + r->params[0].offset);
    unsigned long size = r->params[1].value;
    
    assert(size <= count);
    if (size && data && buf) {
        memcpy(buf, data, size);
    }
    
    return size;
}

unsigned long kapi_stdout_write(unsigned long console_id, void *buf, unsigned long count)
{
    msg_t *s = kapi_msg(KAPI_STDOUT_WRITE);
    msg_t *r;
    
    msg_param_value(s, console_id);
    msg_param_buffer(s, buf, (size_t)count);
    msg_param_value(s, count);
    
    r = syscall_request();
    return kapi_return_value(r);
}

unsigned long kapi_stderr_write(unsigned long console_id, void *buf, unsigned long count)
{
    msg_t *s = kapi_msg(KAPI_STDERR_WRITE);
    msg_t *r;
    
    msg_param_value(s, console_id);
    msg_param_buffer(s, buf, (size_t)count);
    msg_param_value(s, count);
    
    r = syscall_request();
    return kapi_return_value(r);
}
