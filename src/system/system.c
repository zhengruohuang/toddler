#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "system/include/kapi.h"


static unsigned long thread_test(unsigned long arg)
{
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    kprintf("From thread test, arg: %lu\n", arg);
}

int main(int argc, char *argv[])
{
    int i = 0;
    char buf[64];
    
    kprintf("Toddler system process started!\n");
    
    // Initialize
    init_kapi();
    kprintf("KAPI handlers initialized!\n");
    
    // Thread test
    kthread_t thread;
    for (i = 0; i < 10; i++) {
        kthread_create(&thread, thread_test, (unsigned long)i);
    }
    
    // Do some tests
    do {
        syscall_yield();
        //kprintf("still alive %d\n", i++);
//         struct thread_control_block *tcb = get_tcb();
//         msg_t *msg = syscall_msg();
//         
//         kprintf("User process iteration: %d, TCB: %p, Proc ID: %p, Thread ID: %p, CPU ID: %d, Msg: %p\n", i++,
//                     tcb, tcb->proc_id, tcb->thread_id, tcb->cpu_id, msg);
//         
//         ksnprintf(buf, 64, "This to be displayed in the msg handler!\n");
//         kapi_file_write(0, buf, 64);
    } while (1);
    
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
