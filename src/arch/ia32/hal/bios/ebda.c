#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"


ulong ebda_phys_addr;


void init_ebda()
{
    kprintf("Extended BIOS Data Area\n");
    
    u16 ebda_base_addr_from_eda = *((u16 *)POINTER_TO_EBDA_ADDR);
    ebda_phys_addr = ((ulong)ebda_base_addr_from_eda) << 4;
    
    kprintf("\tEBDA Base Address: %p\n", ebda_phys_addr);
}
