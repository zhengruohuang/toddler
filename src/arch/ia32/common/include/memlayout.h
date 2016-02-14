#ifndef __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__


/*
 * Loader
 */
#define LOADER_BASE_PADDR               (0)
#define LOADER_PARAM_ADDR_OFFSET        (0xb000)
#define LOADER_VARIABLES_ADDR_OFFSET    (0xf000)

#define KERNEL_PDE_PFN                  (0)
#define KERNEL_PDE_PADDR                (0)
#define KERNEL_PTE_LO4_PFN              (0)
#define KERNEL_PTE_LO4_PADDR            (0)
#define KERNEL_PTE_HI4_PFN              (0)
#define KERNEL_PTE_HI4_PADDR            (0)

#define HI4_START_VADDR                 (0)

#define HAL_EXEC_START_PFN              (0)
#define KERNEL_EXEC_START_PFN           (0)


/*
 * Boot
 */
#define BOOT_PARAM_ADDRESS_OFFSET       (0)
#define BOOT_PARAM_PADDR                (0)

#define BOOT_SECTOR_BUFFER_OFFSET_1     (0x5000)
#define BOOT_SECTOR_BUFFER_OFFSET_2     (0x6000)

#define LOADER_LOAD_SEGMENT             (0x0)
#define LOADER_LOAD_OFFSET              (0xc000)
#define LOADER_LOAD_PADDR               ((LOADER_LOAD_SEGMENT << 4) | LOADER_LOAD_OFFSET)

#define COREIMG_LOAD_SEGMENT            (0x1000)
#define COREIMG_LOAD_OFFSET             (0x0)
#define COREIMG_LOAD_PADDR              ((COREIMG_LOAD_SEGMENT << 4) | COREIMG_LOAD_OFFSET)

#define BOOT_ENTRY_SEGMENT              (0)
#define BOOT_ENTRY_OFFSET               (0x7c00)


#endif
