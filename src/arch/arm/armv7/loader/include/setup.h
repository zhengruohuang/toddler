#ifndef __ARCH_ARMV7_LOADER_INCLUDE_SETUP__
#define __ARCH_ARMV7_LOADER_INCLUDE_SETUP__


#include "common/include/data.h"


extern void setup_paging(ulong page_dir, ulong dram_start_paddr, ulong dram_end_paddr, ulong hal_area_base_paddr);
extern void enable_mmu(ulong page_dir);
extern void enable_caches();
extern void enable_bpred();

extern void call_hal_entry(ulong entry, ulong stack, ulong bp);


#endif
