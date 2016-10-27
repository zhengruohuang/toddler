#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "klibc/include/kthread.h"
#include "klibc/include/sys.h"
#include "driver/include/console.h"


int stdin_write(unsigned long console_id, char *buf, size_t size)
{
    struct console *con = get_console(console_id);
    if (!con) {
        return 0;
    }
    
    kthread_mutex_lock(&con->stdin_mutex);
    
    if (con->stdin_buf.index + (int)size <= con->stdin_buf.size) {
        memcpy(&con->stdin_buf.data[con->stdin_buf.index], buf, size);
        con->stdin_buf.index += (int)size;
        atomic_membar();
    }
    
    kthread_mutex_unlock(&con->stdin_mutex);
    
    return (int)size;
}

static void print_out_buffer(char *buf, unsigned long count)
{
    if (buf[count - 1] == '\0') {
        kprintf(buf);
    } else {
        char last[2] = { '\0', '\0' };
        
        last[0] = buf[count - 1];
        buf[count - 1] = '\0';
        
        kprintf(buf);
        kprintf(last);
    }
}

static asmlinkage void kapi_stdin_read_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    
    unsigned long console_id = msg->params[0].value;
    struct console *con = get_console(console_id);
    
    unsigned long buf_size = msg->params[1].value;
    
    // Setup the msg
    msg_t *s = syscall_msg();
    s->mailbox_id = reply_mbox_id;
    
    if (con) {
        while (!con->stdin_buf.index) {
            sys_yield();
            atomic_membar();
        }
        
        kthread_mutex_lock(&con->stdin_mutex);
        
        if (buf_size >= con->stdin_buf.index) {
            msg_param_buffer(s, con->stdin_buf.data, con->stdin_buf.index);
            msg_param_value(s, (unsigned long)con->stdin_buf.index);
            con->stdin_buf.index = 0;
        } else {
            msg_param_buffer(s, con->stdin_buf.data, buf_size);
            msg_param_value(s, buf_size);
            con->stdin_buf.index -= buf_size;
            memcpy(con->stdin_buf.data, con->stdin_buf.data + buf_size, con->stdin_buf.index);
        }
        
        kthread_mutex_unlock(&con->stdin_mutex);
    } else {
        msg_param_buffer(s, NULL, 0);
        msg_param_value(s, 0);
    }
    
    // Issue the reply
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

static asmlinkage void kapi_stdout_write_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    unsigned long printed_count = 0;
    msg_t *s = NULL;
    
    // Process the msg
    unsigned long console_id = (int)msg->params[0].value;
    char *buf = (char *)((unsigned long)msg + msg->params[1].offset);
    unsigned long count = msg->params[2].value;
    
    struct console *con = get_console(console_id);
    if (con && con->activated) {
        print_out_buffer(buf, count);
        printed_count = count;
    }
    
    // Setup reply msg
    s = syscall_msg();
    s->mailbox_id = reply_mbox_id;
    msg_param_value(s, printed_count);
    
    // Issue the reply
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

static asmlinkage void kapi_stderr_write_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    unsigned long printed_count = 0;
    msg_t *s = NULL;
    
    // Process the msg
    unsigned long console_id = (int)msg->params[0].value;
    char *buf = (char *)((unsigned long)msg + msg->params[1].offset);
    unsigned long count = msg->params[2].value;
    
    struct console *con = get_console(console_id);
    if (con && con->activated) {
        print_out_buffer(buf, count);
        printed_count = count;
    }
    
    // Setup reply msg
    s = syscall_msg();
    s->mailbox_id = reply_mbox_id;
    msg_param_value(s, printed_count);
    
    // Issue the reply
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}


void init_stdio_kapi()
{
    kapi_reg(KAPI_STDIN_READ, kapi_stdin_read_handler);
    kapi_reg(KAPI_STDOUT_WRITE, kapi_stdout_write_handler);
    kapi_reg(KAPI_STDERR_WRITE, kapi_stderr_write_handler);
}
