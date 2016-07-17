#include "common/include/data.h"
#include "common/include/atomic.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/kthread.h"


extern int main(int argc, char *argv[]);


static void klib_init()
{
    init_tls();
    init_kthread();
    
    init_halloc();
    init_salloc();
    init_malloc();
    //test_malloc();
}

asmlinkage void _start()
{
    kprintf("Klib started!\n");
    
    // Initialize klib
    klib_init();
    
    // Make all writes effective
    atomic_membar();
    
    // Call the main function
    main(0, NULL);
    
    // Done
    sys_unreahable();
}

void klib_init_thread()
{
}
