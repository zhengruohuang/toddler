#include "common/include/data.h"
#include "common/include/errno.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/sys.h"
#include "shell/include/shell.h"


int pwd(int argc, char **argv)
{
    char *cwd = get_cwd();
    kprintf("%s\n", cwd);
    free(cwd);
    
    return EOK;
}
