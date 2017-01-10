#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "kernel/include/hal.h"
#include "kernel/include/syscall.h"


no_opt struct thread_control_block *ksys_get_tcb()
{
    unsigned long addr = hal->kget_tcb();
    return (struct thread_control_block *)addr;
}

int ksys_syscall(unsigned long num, unsigned long param1, unsigned long param2, unsigned long *out1, unsigned long *out2)
{
//     kprintf("To invoke syscall @ %p\n", hal->ksyscall);
    return hal->ksyscall(num, param1, param2, out1, out2);
}

msg_t *ksys_msg()
{
    struct thread_control_block *tcb = ksys_get_tcb();
    if (!tcb) {
        return NULL;
    }
    
    msg_t *msg = tcb->msg_send;
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
    struct thread_control_block *tcb = ksys_get_tcb();
    if (!tcb) {
        return NULL;
    }
    
    msg_t *msg = tcb->msg_recv;
    return msg;
}

msg_t *ksys_request()
{
    int succeed = ksys_syscall(SYSCALL_REQUEST, 0, 0, NULL, NULL);
//     kprintf("ksys request: %d\n", succeed);
//     if (succeed) {
        return get_recv_msg();
//     } else {
//         return NULL;
//     }
}
