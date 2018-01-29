#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"
#include "hal/include/periph.h"


int num_cpus = 0;


void init_topo()
{
    kprintf("Detecting processor topology\n");
    
    num_cpus = periph_detect_num_cpus();
    
    kprintf("\tNumber of logical CPUs: %d\n", num_cpus);
}
