#include "common/include/data.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"
#include "klibc/include/kthread.h"


extern int main(int argc, char *argv[]);


asmlinkage void _start()
{
    kprintf("Klibc started!\n");
    
    init_tls();
    init_kthread();
    
    main(0, NULL);
    
    sys_unreahable();
}
