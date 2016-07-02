#include "common/include/data.h"
#include "klibc/include/stdio.h"


int main(int argc, char *argv[])
{
    kprintf("User process started!\n");
    return 0;
}

// asmlinkage void _start()
// {
//     int i = 0;
//     char buf[256];
//     
//     syscall_kputs("User process started!\n");
//     
//     kapi_init();
// 
//     do {
//         __asm__ __volatile__
//         (
//             "pause;"
//             :
//             :
//         );
//         
//         struct thread_control_block *tcb = get_tcb();
//         msg_t *msg = syscall_msg();
//         
//         //syscall_kputs("Am i here?\n");
//         
//         vsnprintf(buf, 256, "User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
//                   tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, msg);
//         syscall_kputs(buf);
//         
//         vsnprintf(buf, 256, "This to be displayed in the msg handler!\n");
//         kapi_write(0, buf, 256);
//         
//         syscall_kputs("USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n");
//     } while (1);
// }
