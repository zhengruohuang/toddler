#ifndef __HAL_INCLUDE_BOOTPARAM__
#define __HAL_INCLUDE_BOOTPARAM__


#include "common/include/data.h"


/*
 * Boot parameters
 */
extern struct boot_parameters *get_bootparam();
extern void init_bootparam(struct boot_parameters *bootparam);


#endif
