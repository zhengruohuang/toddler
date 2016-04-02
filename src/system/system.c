#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"


int a = 0;


no_opt struct thread_control_block *get_tcb()
{
    ulong addr = 0;
    
    __asm__ __volatile__
    (
        "xorl   %%esi, %%esi;"
        "movl   %%gs:(%%esi), %%edi;"
        : "=D" (addr)
        :
    );
    
    return (struct thread_control_block *)addr;
}

no_opt int do_syscall(ulong num, ulong param, ulong *addr_out, ulong *size_out)
{
    int succeed = 0;
    ulong addr = 0;
    ulong size = 0;
    
    __asm__ __volatile__
    (
        "movl   %%esp, %%ecx;"
        "lea   _sysenter_ret, %%edx;"

        "sysenter;"
        
        ".align 4;"
        "_sysenter_ret:;"
        : "=a" (succeed), "=S" (addr), "=D" (size)
        : "S" (num), "D" (param)
    );
    
    if (addr_out) {
        *addr_out = addr;
    }
    
    if (size_out) {
        *size_out = size;
    }
    
    return succeed;
}

int syscall_ping(ulong ping, ulong *pong)
{
    return do_syscall(SYSCALL_PING, ping, pong, NULL);
}

int syscall_kputs(char *s)
{
    return do_syscall(SYSCALL_KPUTS, (ulong)s, NULL, NULL);
}

ipc_msg_t *syscall_msg()
{
    struct thread_control_block *tcb = get_tcb();
    if (!tcb) {
        return NULL;
    }
    
    struct ipc_msg *msg = tcb->msg;
    msg->dest_mailbox_id = IPC_DEST_KERNEL;
    msg->func_num = IPC_FUNC_KAPI;
    msg->func_param = KAPI_NONE;
    msg->need_reply = 1;
    msg->content_offset = sizeof(struct ipc_msg);
    
    return msg;
}

int syscall_send(ipc_msg_t *msg)
{
    //assert(msg == syscall_msg());
    return 0;
}

ipc_msg_t *syscall_sendrecv(ipc_msg_t *msg)
{
    return NULL;
}

ipc_msg_t *syscall_recv()
{
    return NULL;
}


int kapi_write(int fd, const void *buf, size_t count)
{
    // Setup the msg header
    ipc_msg_t *s = syscall_msg();
    s->func_param = KAPI_WRITE;
    
    // Setup the request
    struct msg_kapi_write_req *req = (struct msg_kapi_write_req *)((unsigned long)s + s->content_offset);
    req->fd = fd;
    req->buf = (void *)buf;
    req->count = count;
    
    // Obtain the reply
    ipc_msg_t *r = syscall_sendrecv(s);
    struct msg_kapi_write_reply *reply = (struct msg_kapi_write_reply *)((unsigned long)r + r->content_offset);
    
    // Done
    return reply->count;
}


extern int asmlinkage vsnprintf(char *buf, size_t size, char *fmt, ...);

static char buf[256];

asmlinkage void _start()
{
    int i = 0;

    do {
        __asm__ __volatile__
        (
            "pause;"
            :
            :
        );
        
        struct thread_control_block *tcb = get_tcb();
        ipc_msg_t *msg = syscall_msg();
        
        vsnprintf(buf, 256, "User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
                  tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, msg);
        
        
        
        syscall_kputs(buf);
        syscall_kputs("USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
    } while (1);
}
