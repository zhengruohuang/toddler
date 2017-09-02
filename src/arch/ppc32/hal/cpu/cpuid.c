#include "common/include/data.h"
#include "common/include/ofw.h"
#include "hal/include/periph.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"


/*
 * CPU properties
 */
struct cpu_prop cpu_info;


/*
 * PVR
 */
static struct pvr_record pvr_table[] = {
    { 0, "Unknown", "Unknown", "Unknown" },
    { 0x00010001, "PowerPC", "601", "0.1" },
    { 0x000c0209, "G4", "7400", "2.9" },
    { 0x00390202, "G5", "970", "2.2" },
    { 0x003c0300, "G5", "970FX", "3.0" },
};

static struct pvr_record *record;

static ulong read_pvr()
{
    ulong pvr = 0;
    
    __asm__ __volatile__
    (
        "mfspr %[dest], 287;"
        : [dest]"=r"(pvr)
        :
    );
    
    return pvr;
}

static int decode_pvr(ulong pvr)
{
    int i;
    for (i = 0; i < sizeof(pvr_table) / sizeof(struct pvr_record); i++) {
        if (pvr_table[i].pvr == pvr) {
            return i;
        }
    }
    
    return 0;
}


/*
 * Init
 */
void init_cpuid()
{
    // PVR
    ulong pvr = 0;
    
    kprintf("Detecting CPU information\n");
    
    pvr = read_pvr();
    record = &pvr_table[decode_pvr(pvr)];
    
    kprintf("\tPVR: %p, class: %s, model: %s, revision: %s\n",
        (void *)pvr, record->class, record->model, record->rev
    );
    
    // CPU info
    struct ofw_tree_prop *prop = NULL;
    struct ofw_tree_node *node = ofw_node_find_by_name(NULL, "cpus");
    node = ofw_node_get_child(node);
    
    prop = ofw_prop_find(node, "timebase-frequency");
    cpu_info.freq_base = *(u32 *)prop->value;
    
    prop = ofw_prop_find(node, "clock-frequency");
    cpu_info.freq_clock = *(u32 *)prop->value;
    
    prop = ofw_prop_find(node, "bus-frequency");
    cpu_info.freq_bus = *(u32 *)prop->value;
    
    kprintf("\tFrequency base: %p, clock: %p, bus: %p\n",
        (void *)cpu_info.freq_base,
        (void *)cpu_info.freq_clock,
        (void *)cpu_info.freq_bus
    );
}
