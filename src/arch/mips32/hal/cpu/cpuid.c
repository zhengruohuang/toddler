#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


void init_cpuid()
{
    u32 prid = 0;
    __asm__ __volatile__ (
        "mfc0   %0, $15;"
        : "=r" (prid)
        :
    );
    
    u32 rev = prid & 0xff;
    prid >>= 8;
    
    u32 proc_id = prid & 0xff;
    prid >>= 8;
    
    u32 comp_id = prid & 0xff;
    prid >>= 8;
    
    u32 comp_opt = prid & 0xff;
    
    kprintf("CPU ID detected, Company ID: %x, Option: %x, Processor ID: %x, Revision: %x\n", comp_id, comp_opt, proc_id, rev);
}
