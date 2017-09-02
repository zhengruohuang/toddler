#include "common/include/data.h"
#include "common/include/bootparam.h"


static struct boot_parameters *boot_param;


struct boot_parameters *get_bootparam()
{
    return boot_param;
}

void init_bootparam(struct boot_parameters *bootparam)
{
    boot_param = bootparam;
}
