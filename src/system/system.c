#include "common/include/data.h"
#include "klibc/include/stdio.h"
#include "klibc/include/sys.h"
#include "klibc/include/kthread.h"
#include "system/include/kapi.h"
#include "system/include/urs.h"


static unsigned long thread_test(unsigned long arg)
{
    __asm__ __volatile__
    (
        "xchgw %%bx, %%bx;"
        :
        :
    );
    
    kprintf("From thread test, arg: %lu\n", arg);
    return 0;
}

int main(int argc, char *argv[])
{
    int i = 0;
    char buf[64];
    
    kprintf("Toddler system process started!\n");
    
    // Initialize
    init_kapi();
    init_urs();
    
    // Init done
    kprintf("System process initialized!\n");
    kapi_process_started(0);
    
    // Thread test
    kthread_t thread;
    for (i = 0; i < 2; i++) {
        kthread_create(&thread, thread_test, (unsigned long)i);
    }
    
    // Block here
    do {
        syscall_yield();
    } while (1);
    
    return 0;
}
