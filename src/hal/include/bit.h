#ifndef __HAL_INCLUDE_BIT__
#define __HAL_INCLUDE_BIT__


#include "common/include/data.h"


/*
 * Alignment
 */
#ifndef ALIGN_UP
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(s, a)  ((s) & ~((a) - 1))
#endif


/*
 * First non-zero bit from the left
 */
extern int fnzb32(u32 arg);
extern int fnzb64(u64 arg);


#endif
