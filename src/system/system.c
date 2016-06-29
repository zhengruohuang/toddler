#include "common/include/data.h"
#include "common/include/syscall.h"
#include "common/include/proc.h"
#include "system/include/syscall.h"
#include "system/include/kapi.h"


int a = 0;


extern int asmlinkage vsnprintf(char *buf, size_t size, char *fmt, ...);

//static 

int asmlinkage var_test(char *buf, size_t size, char *fmt, ...)
{
    char *c = fmt;
    int ftype, ft_count, param_size, prefix, upper, has_sign;
    
    u32 arg4 = 0;
    u64 arg8 = 0;
    ulong va = (ulong)&fmt + sizeof(char *);
    
    int cur_index = 0;
    
    buf[0] = 'a';
    buf[1] = '\n';
    buf[2] = '\0';
    
    syscall_kputs(buf);
}

asmlinkage void _start()
{
    int i = 0;
    char buf[256];
    
    syscall_kputs("User process started!\n");
    
    kapi_init();

    do {
        __asm__ __volatile__
        (
            "pause;"
            :
            :
        );
        
        struct thread_control_block *tcb = get_tcb();
        msg_t *msg = syscall_msg();
        
        //syscall_kputs("Am i here?\n");
        
        vsnprintf(buf, 256, "User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
                  tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, msg);
        syscall_kputs(buf);
        
        vsnprintf(buf, 256, "This to be displayed in the msg handler!\n");
        kapi_write(0, buf, 256);
        
        syscall_kputs("USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
    } while (1);
}
