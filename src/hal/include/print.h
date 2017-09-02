#ifndef __HAL_INCLUDE_PRINT__
#define __HAL_INCLUDE_PRINT__


#include "common/include/data.h"


#define TAB_WIDTH   4


extern asmlinkage int kprintf(char *fmt, ...) __attribute__((format(printf, 1, 2)));


#endif
