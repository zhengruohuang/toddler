#include "common/include/data.h"
#include "common/include/reg.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


static void init_proc_id()
{
    struct cp0_proc_id id;
    read_cp0_proc_id(id.value);
    
    u32 rev = id.rev;
    u32 proc_id = id.proc_id;
    u32 comp_id = id.comp_id;
    u32 comp_opt = id.comp_opt;
    
    kprintf("CPU ID detected, Company ID: %x, Option: %x, Processor ID: %x, Revision: %x\n", comp_id, comp_opt, proc_id, rev);
}

int read_cpu_config(int index, u32 *value)
{
    int success = 0;
    struct cp0_config conf;
    conf.value = 0;
    
    success = 1;
    read_cp0_config(0, conf.value);
    if (index == 0) goto __done;
    
    success = conf.has_next;
    if (!success) goto __done;
    read_cp0_config(1, conf.value);
    if (index == 1) goto __done;
    
    success = conf.has_next;
    if (!success) goto __done;
    read_cp0_config(2, conf.value);
    if (index == 2) goto __done;
    
    success = conf.has_next;
    if (!success) goto __done;
    read_cp0_config(3, conf.value);
    if (index == 3) goto __done;
    
    success = conf.has_next;
    if (!success) goto __done;
    read_cp0_config(4, conf.value);
    if (index == 4) goto __done;
    
    success = conf.has_next;
    if (!success) goto __done;
    read_cp0_config(5, conf.value);
    if (index == 5) goto __done;
    
    success = 0;
    goto __done;
    
__done:
    if (value) {
        *value = conf.value;
    }
    
    return success;
}


void init_cpuid()
{
    init_proc_id();
}
