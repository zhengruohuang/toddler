#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "klibc/include/sys.h"


int syscall_ping(unsigned long ping, unsigned long *pong)
{
    return do_syscall(SYSCALL_PING, ping, 0, pong, NULL);
}

int syscall_kputs(char *s)
{
    return do_syscall(SYSCALL_KPUTS, (unsigned long)s, 0, NULL, NULL);
}

int syscall_time(unsigned long *high, unsigned long *low)
{
    return do_syscall(SYSCALL_TIME, 0, 0, high, low);
}

int syscall_yield()
{
    return do_syscall(SYSCALL_YIELD, 0, 0, NULL, NULL);
}

msg_t *syscall_msg()
{
    struct thread_control_block *tcb = get_tcb();
    //kprintf("tcb @ %p\n", tcb);
    if (!tcb) {
        return NULL;
    }
    
    msg_t *msg = tcb->msg_send;
    //kprintf("Msg @ %p\n", msg);
    
    msg->mailbox_id = IPC_MAILBOX_NONE;
    msg->opcode = IPC_OPCODE_NONE;
    msg->func_num = 0;
    msg->param_count = 0;
    
    msg->msg_size = (int)sizeof(msg_t);
    if (msg->msg_size % (int)sizeof(unsigned long)) {
        msg->msg_size /= (int)sizeof(unsigned long);
        msg->msg_size++;
        msg->msg_size *= (int)sizeof(unsigned long);
    }
    
    return msg;
}

static msg_t *get_recv_msg()
{
    struct thread_control_block *tcb = get_tcb();
    if (!tcb) {
        return NULL;
    }
    
    msg_t *msg = tcb->msg_recv;
    return msg;
}

int syscall_send()
{
    int succeed = do_syscall(SYSCALL_SEND, 0, 0, NULL, NULL);
    return succeed;
}

int syscall_reply()
{
    int succeed = do_syscall(SYSCALL_REPLY, 0, 0, NULL, NULL);
    return succeed;
}

msg_t *syscall_recv()
{
    int succeed = do_syscall(SYSCALL_RECV, 0, 0, NULL, NULL);
    if (succeed) {
        return get_recv_msg();
    } else {
        return NULL;
    }
}

msg_t *syscall_request()
{
    int succeed = do_syscall(SYSCALL_REQUEST, 0, 0, NULL, NULL);
    if (succeed) {
        return get_recv_msg();
    } else {
        return NULL;
    }
}

int syscall_respond()
{
    int succeed = do_syscall(SYSCALL_RESPOND, 0, 0, NULL, NULL);
    return succeed;
}

int syscall_reg_msg_handler(unsigned long msg_num, msg_handler_t msg_handler)
{
    int succeed = do_syscall(SYSCALL_REG_MSG_HANDLER, msg_num, (unsigned long)msg_handler, NULL, NULL);
    return succeed;
}

int syscall_unreg_msg_handler(unsigned long msg_num)
{
    int succeed = do_syscall(SYSCALL_UNREG_MSG_HANDLER, msg_num, 0, NULL, NULL);
    return succeed;
}

int syscall_reg_kapi_server(unsigned long kapi_num)
{
    int succeed = do_syscall(SYSCALL_REG_KAPI_SERVER, kapi_num, 0, NULL, NULL);
    return succeed;
}

int syscall_unreg_kapi_server(unsigned long kapi_num)
{
    int succeed = do_syscall(SYSCALL_UNREG_KAPI_SERVER, kapi_num, 0, NULL, NULL);
    return succeed;
}
