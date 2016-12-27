#include "common/include/data.h"
#include "hal/include/print.h"


void no_opt halt()
{
    while (1);
}

void disp_src_info(char *file, char *base, int line)
{
    kprintf("[SRC] File: %s, Base: %s, Line: %d\n", file, base, line);
}
