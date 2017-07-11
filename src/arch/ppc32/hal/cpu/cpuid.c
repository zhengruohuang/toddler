#include "common/include/data.h"
#include "hal/include/print.h"


struct pvr_record {
    ulong pvr;
    char *class;
    char *model;
    char *rev;
};


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

void init_cpuid()
{
    ulong pvr = 0;
    
    kprintf("Detecting CPU version\n");
    
    pvr = read_pvr();
    record = &pvr_table[decode_pvr(pvr)];
    
    kprintf("\tPVR: %p, class: %s, model: %s, revision: %s\n",
        (void *)pvr, record->class, record->model, record->rev
    );
}
