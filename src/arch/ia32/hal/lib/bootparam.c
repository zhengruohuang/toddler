#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/memlayout.h"

struct boot_parameters *get_bootparam()
{
    return (struct boot_parameters *)BOOT_PARAM_PADDR;
}
