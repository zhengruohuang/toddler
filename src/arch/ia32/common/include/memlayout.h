#ifndef __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__


#include "common/include/memory.h"


/*
 * HAL
 */
#define POINTER_TO_EBDA_ADDR            (0x40e)

#define LAPIC_DEFAULT_PADDR             (0xfee00000)
#define IOAPIC_DEFAULT_PADDR            (0xfec00000)
#define LAPIC_VADDR                     (0xFFFFC000)
#define IOAPIC_TOP_VADDR                (0xFFFFC000)

#define PER_CPU_AREA_PAGE_COUNT         (1)
#define PER_CPU_AREA_SIZE               (PAGE_SIZE * PER_CPU_AREA_PAGE_COUNT)

#define PER_CPU_AREA_TOP_VADDR          (0xFFF00000)
#define PER_CPU_AREA_STACK_TOP_OFFSET   (PER_CPU_AREA_SIZE)
#define PER_CPU_AREA_DATA_START_OFFSET  (0)


/*
 * Loader
 */
#define LOADER_VARIABLES_ADDR_SEGMENT   (0x0)
#define LOADER_VARIABLES_ADDR_OFFSET    (0xf000)
#define LOADER_VARIABLES_PADDR          (0xf000)

#define KERNEL_PDE_PFN                  (383)
#define KERNEL_PDE_PADDR                (KERNEL_PDE_PFN << 12)
#define KERNEL_PTE_LO4_PFN              (382)
#define KERNEL_PTE_LO4_PADDR            (KERNEL_PTE_LO4_PFN << 12)
#define KERNEL_PTE_HI4_PFN              (381)
#define KERNEL_PTE_HI4_PADDR            (KERNEL_PTE_HI4_PFN << 12)

#define KERNEL_INIT_PTE_START_PFN       (512)
#define KERNEL_INIT_PTE_START_PADDR     (KERNEL_INIT_PTE_START_PFN << 12)

#define HI4_START_VADDR                 (0xFFC00000)

#define HAL_EXEC_START_PFN              (256)
#define KERNEL_EXEC_START_PFN           (384)

#define HAL_SPACE_END_VADDR             (0xFFFFD000)

#define TEXT_VIDEO_MEM_ADDR             (0xb8000)


/*
 * Boot
 */
#define BOOT_PARAM_ADDR_OFFSET          (0x2000)
#define BOOT_PARAM_PADDR                (0x2000)

#define BOOT_SECTOR_BUFFER_OFFSET_1     (0x5000)
#define BOOT_SECTOR_BUFFER_OFFSET_2     (0x6000)

#define LOADER_LOAD_SEGMENT             (0x0)
#define LOADER_LOAD_OFFSET              (0xb000)
#define LOADER_LOAD_PADDR               ((LOADER_LOAD_SEGMENT << 4) | LOADER_LOAD_OFFSET)

#define COREIMG_LOAD_SEGMENT            (0x1000)
#define COREIMG_LOAD_OFFSET             (0x0)
#define COREIMG_LOAD_PADDR              ((COREIMG_LOAD_SEGMENT << 4) | COREIMG_LOAD_OFFSET)

#define BOOT_ENTRY_SEGMENT              (0)
#define BOOT_ENTRY_OFFSET               (0x7c00)


#endif
