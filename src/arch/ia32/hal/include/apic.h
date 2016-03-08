#ifndef __ARCH_IA32_HAL_INCLUDE_APIC__
#define __ARCH_IA32_HAL_INCLUDE_APIC__


#include "common/include/data.h"


extern int apic_supported;
extern int apic_present;
extern ulong lapic_paddr;

extern void init_apic();


#endif
