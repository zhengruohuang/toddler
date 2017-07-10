#ifndef __ARCH_PPC32_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_PPC32_COMMON_INCLUDE_MEMLAYOUT__


#include "common/include/memory.h"


/*
 * Loader
 */
#define LOADER_MEMPOOL_SIZE                 (65536)
#define LOADER_PHT_SIZE                     (65536)
#define LOADER_PHT_ATTRI_SIZE               (LOADER_PHT_SIZE / sizeof(struct pht_entry) * sizeof(struct pht_attri_entry))


/*
 * HAL
 */
#define HAL_AREA_SIZE                       (0x100000)      // 1MB
#define HAL_AREA_VADDR                      (0xfff00000)    // 4GB-4MB
#define HAL_VSPACE_END                      (0xfffff000)    // 4GB-4KB

#define HAL_BASE_VADDR                      (0xfff80000)

#define HAL_STACK_TOP_OFFSET                (0x8000 - 0x10) // 32KB
#define HAL_STACK_TOP_VADDR                 (HAL_BASE_VADDR + HAL_STACK_TOP_OFFSET)

#define PER_CPU_AREA_BASE_VADDR             (0xffc00000)    // 4GB-4MB

#define PER_CPU_AREA_PAGE_COUNT             (2)
#define PER_CPU_AREA_SIZE                   (PAGE_SIZE * PER_CPU_AREA_PAGE_COUNT)
#define PER_CPU_DATA_START_OFFSET           (0)
#define PER_CPU_STACK_TOP_OFFSET            (PER_CPU_AREA_SIZE - 0x10)

#define THREAD_CTRL_BLOCK_ALIGNMENT         (64)


/*
 * User address space
 */
#define USER_VADDR_SPACE_END                (0xf0000000)  // 4GB-256MB, the last segment is for kernel


#endif

