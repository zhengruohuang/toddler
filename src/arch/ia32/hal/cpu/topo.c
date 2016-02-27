#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


static int num_cpus;


int get_num_cpus()
{
    assert(num_cpus >= 1);
    return num_cpus;
}

static void detect_topo()
{
}

void init_topo()
{
    kprintf("Detecting processor topology\n");
    detect_topo();
}
