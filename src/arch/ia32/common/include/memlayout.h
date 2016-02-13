#ifndef __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__
#define __ARCH_IA32_COMMON_INCLUDE_MEMLAYOUT__


/*
 * Loader
 */
#define LOADER_PARAM_ADDRESS_OFFSET     (0xb000)
#define LOADER_VARIABLES_ADDRESS_OFFSET (0xf000)


/*
 * Boot
 */
#define BOOT_PARAM_ADDRESS_OFFSET       (0)
#define BOOT_PARAM_PADDR                0

#define BOOT_SECTOR_BUFFER_OFFSET_1     0x5000
#define BOOT_SECTOR_BUFFER_OFFSET_2     0x6000

#define LOADER_LOAD_SEGMENT             0x0
#define LOADER_LOAD_OFFSET              0xc000
#define COREIMG_LOAD_SEGMENT            0x1000
#define COREIMG_LOAD_OFFSET             0x0

#define BOOT_ENTRY_SEGMENT              0
#define BOOT_ENTRY_OFFSET               0x7c00


#endif
