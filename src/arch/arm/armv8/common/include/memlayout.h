#ifndef __ARCH_ARMV7_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_ARMV7_COMMON_INCLUDE_MEMLAYOUT__


#include "common/include/memory.h"


/*
 * Loader
 */
// #define LOADER_MEMPOOL_SIZE                 (0x10000)       // 64KB

/*
 * HAL
 */
#define HAL_VSPACE_END                      (0xffff0000)    // 4GB-64KB, interrupt vector starts @ 0xffff0000
#define HAL_L1TABLE_OFFSET                  (0xf8000)       // 1MB-32KB

#define HAL_STACK_TOP_VADDR                 (0xfff88000 - 0x10) // 32KB

#define PER_CPU_AREA_BASE_VADDR             (0xffc00000)    // 4GB-4MB

#define PER_CPU_AREA_PAGE_COUNT             (2)
#define PER_CPU_AREA_SIZE                   (PAGE_SIZE * PER_CPU_AREA_PAGE_COUNT)
#define PER_CPU_DATA_START_OFFSET           (0)
#define PER_CPU_STACK_TOP_OFFSET            (PER_CPU_AREA_SIZE - 0x10)

// #define THREAD_CTRL_BLOCK_ALIGNMENT         (64)


/*
 * User address space
 */
#define USER_VADDR_SPACE_END                (0xf0000000)  // 4GB-256MB


#endif

