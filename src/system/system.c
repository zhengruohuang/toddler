#include "common/include/data.h"
#include "common/include/syscall.h"


int a = 0;


int no_opt do_syscall(ulong num, ulong param, ulong *addr_out, ulong *size_out)
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


void asmlinkage _start()
{
    do {
        __asm__ __volatile__
        (
            "pause;"
            :
            :
        );
        
        char *test_char = "USER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!\n";
        
        syscall_kputs(test_char);
    } while (1);
}
