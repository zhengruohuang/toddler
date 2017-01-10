#ifndef __ARCH_MIPS32_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_MIPS32_COMMON_INCLUDE_MEMLAYOUT__


#include "common/include/memory.h"


/*
 * Segment
 */
#define SEG_USER            0x0
#define SEG_LOW_CACHED      0x80000000
#define SEG_LOW_DIRECT      0xa0000000
#define SEG_KERNEL          0xc0000000


/*
 * Loader
 */
#define LOADER_STACK_TOP    0x10000     // 64KB


/*
 * HAL
 */
#define HAL_STACK_TOP_OFFSET                (0x11000 - 0x10)
#define HAL_STACK_TOP_ADDR                  (SEG_LOW_CACHED + HAL_STACK_TOP_OFFSET)

#define PER_CPU_AREA_PAGE_COUNT             (2)
#define PER_CPU_AREA_SIZE                   (PAGE_SIZE * PER_CPU_AREA_PAGE_COUNT)
#define PER_CPU_DATA_START_OFFSET           (0)
#define PER_CPU_TLB_REFILL_STACK_TOP_OFFSET (PER_CPU_AREA_SIZE - PAGE_SIZE - 0x10)
#define PER_CPU_STACK_TOP_OFFSET            (PER_CPU_AREA_SIZE - 0x10)

#define THREAD_CTRL_BLOCK_ALIGNMENT         (64)


/*
 * User address space
 */
#define USER_VADDR_SPACE_END    0x7f000000


#endif
