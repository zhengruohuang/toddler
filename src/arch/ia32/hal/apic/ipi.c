#include "common/include/data.h"
#include "common/include/task.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/time.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/apic.h"


struct ipi_record {
    int registered;
    int vector;
    int_handler handler;
};


static struct ipi_record ipi_map[256];
static int cur_ipi_id = 0;


static int generic_ipi_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    kprintf("Generic IPI Handler\n");
    return 0;
}

static int get_ipi_vector(int ipi_id)
{
    if (ipi_map[ipi_id].registered) {
        return ipi_map[ipi_id].vector;
    } else {
        panic("IPI %d not registered!", ipi_id);
    }
    
    return 0;
}

void init_ipi()
{
    int i;
    
    for (i = 0; i < sizeof(ipi_map) / sizeof(struct ipi_record); i++) {
        ipi_map[i].vector = 0;
        ipi_map[i].handler = NULL;
        ipi_map[i].registered = 0;
    }
}

int register_ipi(int_handler handler)
{
    int_handler real_handler = handler;
    if (NULL == real_handler) {
        real_handler = generic_ipi_handler;
    }
    
    // First alloc a vector and IPI ID
    int vector = alloc_int_vector(handler);
    int ipi_id = cur_ipi_id++;
    
    //kprintf("Vector %d\n", vector);    
    
    // Then set the IPI record
    ipi_map[ipi_id].vector = vector;
    ipi_map[ipi_id].registered = 1;
    ipi_map[ipi_id].handler = handler;
    
    return ipi_id;
}


/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_send_startup(int apicid)
{
    struct apic_interupt_command_register icr;
    
    kprintf("\tTo send init IPI to: %d\n", apicid);
    
    // Send an INIT IPI
    icr.value_low = lapic_vaddr[APIC_ICR_LO];
    icr.value_high = lapic_vaddr[APIC_ICR_HI];
    
    icr.delmod = APIC_DELMOD_INIT;
    icr.destmod = APIC_DESTMOD_PHYS;
    icr.level = APIC_LEVEL_ASSERT;
    icr.trigger_mode = APIC_TRIGMOD_EDGE;
    icr.shorthand = APIC_SHORTHAND_NONE;
    icr.vector = 0;
    icr.dest = apicid;
    
    lapic_vaddr[APIC_ICR_HI] = icr.value_high;
    lapic_vaddr[APIC_ICR_LO] = icr.value_low;
    
    // Wait for 10ms, according to MP Specification
    blocked_delay(10);
    
    // If there are errors
    if (!check_lapic()) {
        panic("Could send initialization IPI!");
    }
    
    // Check IPI delivery status
    icr.value_low = lapic_vaddr[APIC_ICR_LO];
    if (icr.delivs == APIC_DELIVS_PENDING) {
        kprintf("IPI is pending.\n");
        panic("Could send initialization IPI!");
    }
    
    // Try to send INIT IPI again
    icr.value_low = lapic_vaddr[APIC_ICR_LO];
    icr.value_high = lapic_vaddr[APIC_ICR_HI];
    
    icr.delmod = APIC_DELMOD_INIT;
    icr.destmod = APIC_DESTMOD_PHYS;
    icr.level = APIC_LEVEL_DEASSERT;
    icr.shorthand = APIC_SHORTHAND_NONE;
    icr.trigger_mode = APIC_TRIGMOD_EDGE;
    icr.vector = 0;
    icr.dest = apicid;
    
    lapic_vaddr[APIC_ICR_HI] = icr.value_high;
    lapic_vaddr[APIC_ICR_LO] = icr.value_low;
    
    // Wait 10ms as is specified by MP Specification
    blocked_delay(10);
    
    if (!APIC_IS_LAPIC_82489DX(lapic_vaddr[APIC_LAVR])) {
        // If this is not 82489DX-based lapic_vaddr we must send two STARTUP IPI's
        u32 i;
        for (i = 0; i < 2; i++) {
            icr.value_low = lapic_vaddr[APIC_ICR_LO];
            icr.value_high = lapic_vaddr[APIC_ICR_HI];
            
            icr.vector = (u8)(get_bootparam()->ap_entry_addr >> 12); // calculate the reset vector
            icr.delmod = APIC_DELMOD_STARTUP;
            icr.destmod = APIC_DESTMOD_PHYS;
            icr.level = APIC_LEVEL_ASSERT;
            icr.shorthand = APIC_SHORTHAND_NONE;
            icr.trigger_mode = APIC_TRIGMOD_EDGE;
            icr.dest = apicid;
            lapic_vaddr[APIC_ICR_HI] = icr.value_high;
            lapic_vaddr[APIC_ICR_LO] = icr.value_low;
            
            //According to MP Specification, we should wait for 200us
            blocked_delay(1);
        }
    }
    
    return check_lapic();
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int lapic_vaddr_ipi_broadcast(int self, int ipi_id)
{
    struct apic_interupt_command_register icr;
    
    icr.value_low = lapic_vaddr[APIC_ICR_LO];
    icr.delmod = APIC_DELMOD_FIXED;
    icr.destmod = APIC_DESTMOD_PHYS;
    icr.level = APIC_LEVEL_ASSERT;
    icr.shorthand = self ? APIC_SHORTHAND_ALL_INCL : APIC_SHORTHAND_ALL_EXCL;
    icr.trigger_mode = APIC_TRIGMOD_EDGE;
    icr.vector = get_ipi_vector(ipi_id);
    
    lapic_vaddr[APIC_ICR_LO] = icr.value_low;
    
    icr.value_low = lapic_vaddr[APIC_ICR_LO];
    if (icr.delivs == APIC_DELIVS_PENDING) {
        kprintf("IPI is pending\n");
    }
    
    return check_lapic();
}
