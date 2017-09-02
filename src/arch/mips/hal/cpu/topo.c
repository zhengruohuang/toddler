#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


int num_cpus = 0;


void init_topo()
{
    kprintf("Detecting processor topology\n");
    
    num_cpus = 1;
    
    kprintf("\tNumber of logical CPUs: %d\n", num_cpus);
}
