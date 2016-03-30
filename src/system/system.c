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

struct ipc_msg *syscall_msg()
{
    struct thread_control_block *tcb = get_tcb();
    return tcb->msg;
}

int syscall_send(struct ipc_msg *msg)
{
    return 0;
}

struct ipc_msg *syscall_sendrecv(struct ipc_msg *msg)
{
    return NULL;
}

struct ipc_msg *syscall_recv()
{
    return NULL;
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
        
        vsnprintf(buf, 256, "User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
                  tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, tcb->msg);
        
        syscall_kputs(buf);
        syscall_kputs("USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
    } while (1);
}
