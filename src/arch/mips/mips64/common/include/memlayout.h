#ifndef __ARCH_MIPS64_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_MIPS64_COMMON_INCLUDE_MEMLAYOUT__


#include "common/include/memory.h"


/*
 * Segment
 */
#define SEG_USER            0x0
#define SEG_LOW_CACHED      0xffffffff80000000ul
#define SEG_LOW_DIRECT      0xffffffffa0000000ul
#define SEG_KERNEL          0xc0000000


/*
 * Physical memory window
 */
#define SEG_CACHED          0x9000000000000000ul
#define SEG_DIRECT          0x9800000000000000ul


/*
 * Loader
 */
#define LOADER_STACK_TOP    0x10000     // 64KB


/*
 * PIIX4 south bridge
 */
#define SOUTH_BRIDGE_BASE_ADDR  0x18000000ul


/*
 * HAL
 */
#define HAL_VSPACE_END                      0xffffffff80180000ul

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
