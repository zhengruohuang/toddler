#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"
#include "hal/include/mps.h"
#include "hal/include/cpu.h"


int num_cpus = 0;


void init_topo()
{
    kprintf("Detecting processor topology\n");
    
    if (acpi_supported && madt_supported) {
        num_cpus = madt_lapic_count;
        assert(num_cpus);
    }
    
    else if (mps_supported) {
        num_cpus = mps_lapic_count;
        assert(num_cpus);
    }
    
    else {
        num_cpus = 1;
    }
    
    kprintf("\tNumber of logical CPUs: %d\n", num_cpus);
}
