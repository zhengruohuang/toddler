#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "system/include/syscall.h"


no_opt struct thread_control_block *get_tcb()
{
    unsigned long addr = 0;
    
    __asm__ __volatile__
    (
        "xorl   %%esi, %%esi;"
        "movl   %%gs:(%%esi), %%edi;"
        : "=D" (addr)
        :
    );
    
    return (struct thread_control_block *)addr;
}

no_opt int do_syscall(unsigned long num, unsigned long param, unsigned long *out1, unsigned long *out2)
{
    int succeed = 0;
    unsigned long value1 = 0, value2 = 0;
    
    __asm__ __volatile__
    (
        "movl   %%esp, %%ecx;"
        "lea   _sysenter_ret, %%edx;"
        
        "sysenter;"
        
        ".align 4;"
        "_sysenter_ret:;"
        : "=a" (succeed), "=S" (value1), "=D" (value2)
        : "S" (num), "D" (param)
    );
    
    if (out1) {
        *out1 = value1;
    }
    
    if (out2) {
        *out2 = value2;
    }
    
    return succeed;
}

int syscall_ping(unsigned long ping, unsigned long *pong)
{
    return do_syscall(SYSCALL_PING, ping, pong, NULL);
}

int syscall_kputs(char *s)
{
    return do_syscall(SYSCALL_KPUTS, (unsigned long)s, NULL, NULL);
}

msg_t *syscall_msg()
{
    struct thread_control_block *tcb = get_tcb();
    if (!tcb) {
        return NULL;
    }
    
    msg_t *msg = tcb->msg;
    msg->dest_mailbox_id = IPC_DEST_NONE;
    msg->func_type = IPC_FUNC_NONE;
    msg->func_num = 0;
    msg->need_reply = 0;
    msg->param_count = 0;
    
    return msg;
}

int syscall_send(msg_t *msg)
{
    int succeed = do_syscall(SYSCALL_SEND, (unsigned long)msg, NULL, NULL);
    return succeed;
}

msg_t *syscall_recv()
{
    return NULL;
}

msg_t *syscall_request(msg_t *msg)
{
    return NULL;
}

int syscall_reply(msg_t *in_msg, msg_t *out_msg)
{
    return 0;
}
