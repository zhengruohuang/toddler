#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "system/include/syscall.h"
#include "system/include/kapi.h"


int a = 0;


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
        msg_t *msg = syscall_msg();
        
        vsnprintf(buf, 256, "User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
                  tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, msg);
        
        syscall_kputs(buf);
        syscall_kputs("USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
    } while (1);
}
