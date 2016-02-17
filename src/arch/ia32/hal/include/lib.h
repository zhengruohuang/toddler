#ifndef __ARCH_IA32_HAL_INCLUDE_LIB__
#define __ARCH_IA32_HAL_INCLUDE_LIB__


#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/bootparam.h"


static struct boot_parameters *get_bootparam()
{
    return (struct boot_parameters *)BOOT_PARAM_PADDR;
}

static void no_opt halt()
{
    do {
        __asm__ __volatile__
        (
            "hlt;"
            :
            :
        );
    } while (1);
}


#endif
