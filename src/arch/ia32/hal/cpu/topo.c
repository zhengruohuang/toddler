#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"


static int num_cpus;


int get_num_cpus()
{
    assert(num_cpus >= 1);
    return num_cpus;
}

static void detect_topo()
{
    // Since we don't support acpi or mps yet, just assuem there are 8 logical cpus
    num_cpus = 8;
}

void init_topo()
{
    kprintf("Detecting processor topology ... \n");
    detect_topo();
    kprintf("%d logical CPUs\n", get_num_cpus());
}
