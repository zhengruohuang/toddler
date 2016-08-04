#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/string.h"
#include "klibc/include/sys.h"
#include "driver/include/console.h"


int stdin_write(unsigned long console_id, char *buf, size_t size)
{
    struct console *con = get_console(console_id);
    if (!con) {
        return 0;
    }
    
    if (con->stdin_buf.index + (int)size > con->stdin_buf.size) {
        return 0;
    }
    
    memcpy(&con->stdin_buf.data[con->stdin_buf.index], buf, size);
    con->stdin_buf.index += (int)size;
    
    return (int)size;
}

static void print_out_buffer(char *buf, int count)
{
    char last[2] = { '\0', '\0' };
    
    if (buf[count - 1] == '\0') {
        kprintf(buf);
    } else {
        last[0] = buf[count - 1];
        buf[count - 1] = '\0';
        kprintf(buf);
        kprintf(last);
    }
}

asmlinkage void stdin_read_handler(msg_t *msg)
{
    unsigned long reply_mbox_id = msg->mailbox_id;
    
    unsigned long console_id = (int)msg->params[0].value;
    struct console *con = get_console(console_id);
    
    // Setup the msg
    msg_t *s = syscall_msg();
    s->mailbox_id = reply_mbox_id;
    
    if (con) {
        msg_param_buffer(s, con->stdin_buf.data, con->stdin_buf.index);
        msg_param_value(s, con->stdin_buf.index);
    } else {
        msg_param_buffer(s, NULL, 0);
        msg_param_value(s, 0);
    }
    
    // Issue the KAPI and obtain the result
    syscall_respond();
    
    // Should never reach here
    sys_unreahable();
}

asmlinkage void stdout_write_handler(msg_t *msg)
{
    unsigned long console_id = (int)msg->params[0].value;
    
    char *buf = (char *)((unsigned long)msg + msg->params[1].offset);
    int count = (int)msg->params[2].value;
    
    struct console *con = get_console(console_id);
    if (con && con->activated) {
        print_out_buffer(buf, count);
    }
    
    kapi_thread_exit(NULL);
}

asmlinkage void stderr_write_handler(msg_t *msg)
{
    unsigned long console_id = (int)msg->params[0].value;
    
    char *buf = (char *)((unsigned long)msg + msg->params[1].offset);
    int count = (int)msg->params[2].value;
    
    struct console *con = get_console(console_id);
    if (con && con->activated) {
        print_out_buffer(buf, count);
    }
    
    kapi_thread_exit(NULL);
}
