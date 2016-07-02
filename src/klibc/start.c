#include "common/include/data.h"
#include "klibc/include/sys.h"
#include "klibc/include/stdio.h"


extern int main(int argc, char *argv[]);


asmlinkage void _start()
{
    kprintf("Klibc started!\n");
    
    main(0, NULL);
    
    sys_unreahable();
}
