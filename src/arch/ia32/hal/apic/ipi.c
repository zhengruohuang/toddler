#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/apic.h"


struct ipi_record {
    int                     registered;
    int                     vector;
    hal_interrupt_handler   handler;
};

static struct ipi_record ipi_map[256];
static int cur_ipi_map_index = 0;


static int general_ipi_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    kprintf("General IPI Handler\n");
    return 0;
}

static int get_ipi_vecor(u32 ipi_id)
{
    if (ipi_map[ipi_id].registered) {
        return ipi_map[ipi_id].vector;
    } else {
        //panic("IPI not registered!");
    }
    
    return 0;
}


void init_ipi()
{
    u32 i;
    
    for (i = 0; i < sizeof(ipi_map) / sizeof(struct ipi_record); i++) {
        ipi_map[i].vector = 0;
        ipi_map[i].handler = NULL;
        ipi_map[i].registered = 0;
    }
}

/*
 * Return
 *      IPI ID
 */
int ipi_register(hal_interrupt_handler handler)
{
    hal_interrupt_handler real_handler = handler;
    if (NULL == real_handler) {
        real_handler = general_ipi_handler;
    }
    
    /* First register a vector */
    u32 vector = hal_interrupt_vector_alloc(handler);
    
    //kprintf("Vector %d\n", vector);
    
    /* IPI ID */
    int ipi_id = cur_ipi_map_index;
    cur_ipi_map_index++;
    
    /* Set the IPI map */
    ipi_map[ipi_id].vector = (u8)vector;
    ipi_map[ipi_id].registered = 1;
    ipi_map[ipi_id].handler = handler;
    
    return ipi_id;
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_broadcast(int self, int ipi_id)
{
    s_hal_apic_interupt_command_register icr;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    icr.delmod = HAL_APIC_DELMOD_FIXED;
    icr.destmod = HAL_APIC_DESTMOD_PHYS;
    icr.level = HAL_APIC_LEVEL_ASSERT;
    icr.shorthand = (self ? HAL_APIC_SHORTHAND_ALL_INCL : HAL_APIC_SHORTHAND_ALL_EXCL);
    icr.trigger_mode = HAL_APIC_TRIGMOD_EDGE;
    icr.vector = get_ipi_vecor(ipi_id);
    
    hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    if (icr.delivs == HAL_APIC_DELIVS_PENDING) {
        kprintf("IPI is pending.\n");
    }
    
    return check_lapic();
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_send(int apicid, int ipi_id)
{
    s_hal_apic_interupt_command_register icr;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    icr.delmod = HAL_APIC_DELMOD_FIXED;
    icr.destmod = HAL_APIC_DESTMOD_PHYS;
    icr.level = HAL_APIC_LEVEL_ASSERT;
    icr.shorthand = HAL_APIC_SHORTHAND_NONE;
    icr.trigger_mode = HAL_APIC_TRIGMOD_EDGE;
    icr.vector = get_ipi_vecor(ipi_id);
    
    icr.value_high = hal_apic_lapic[HAL_APIC_ICR_HI];
    icr.dest = apicid;
    
    //kprintf("Target APIC ID %d, Vector %d\n", apicid, hal_apic_lapic_ipi_get_vector(ipi_id));
    
    hal_apic_lapic[HAL_APIC_ICR_HI] = icr.value_high;
    hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    if (icr.delivs == HAL_APIC_DELIVS_PENDING) {
        kprintf("IPI is pending.\n");
    }
    
    return check_lapic();
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_self(int ipi_id)
{
    s_hal_apic_interupt_command_register icr;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    icr.delmod = HAL_APIC_DELMOD_FIXED;
    icr.destmod = HAL_APIC_DESTMOD_PHYS;
    icr.level = HAL_APIC_LEVEL_ASSERT;
    icr.shorthand = HAL_APIC_SHORTHAND_SELF;
    icr.trigger_mode = HAL_APIC_TRIGMOD_EDGE;
    icr.vector = hal_apic_lapic_ipi_get_vector(ipi_id);;
    
    hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
    
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    if (icr.delivs == HAL_APIC_DELIVS_PENDING) {
        kprintf("IPI is pending.\n");
    }
    
    return check_lapic();
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_send_startup(int apicid)
{
    s_hal_apic_interupt_command_register icr;
    
    /* Send an INIT IPI */
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    icr.value_high = hal_apic_lapic[HAL_APIC_ICR_HI];
    
    icr.delmod = HAL_APIC_DELMOD_INIT;
    icr.destmod = HAL_APIC_DESTMOD_PHYS;
    icr.level = HAL_APIC_LEVEL_ASSERT;
    icr.trigger_mode = HAL_APIC_TRIGMOD_LEVEL;
    icr.shorthand = HAL_APIC_SHORTHAND_NONE;
    icr.vector = 0;
    icr.dest = apicid;
    
    hal_apic_lapic[HAL_APIC_ICR_HI] = icr.value_high;
    hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
    
    /* Wait for 10ms, according to MP Specification */
    hal_time_delay(10);
    
    /* If there are errors */
    if (!check_lapic()) {
        panic("Could send initialization IPI!");
    }
    
    /* Check IPI delivery status */
    icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
    if (icr.delivs == HAL_APIC_DELIVS_PENDING) {
        kprintf("IPI is pending.\n");
        panic("Could send initialization IPI!");
    }
    
    /* Try to send INIT IPI again */
    icr.delmod = HAL_APIC_DELMOD_INIT;
    icr.destmod = HAL_APIC_DESTMOD_PHYS;
    icr.level = HAL_APIC_LEVEL_DEASSERT;
    icr.shorthand = HAL_APIC_SHORTHAND_NONE;
    icr.trigger_mode = HAL_APIC_TRIGMOD_LEVEL;
    icr.vector = 0;
    icr.dest = apicid;
    hal_apic_lapic[HAL_APIC_ICR_HI] = icr.value_high;
    hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
    
    /* Wait 10ms as MP Specification specifies */
    hal_time_delay(10);
    
    if (!HAL_APIC_IS_LAPIC_82489DX(hal_apic_lapic[HAL_APIC_LAVR])) {
        /* If this is not 82489DX-based hal_apic_lapic we must send two STARTUP IPI's */
        u32 i;
        for (i = 0; i < 2; i++) {
            icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
            icr.vector = (u8)(hal_bootparam_ap_entry >> 12); /* calculate the reset vector */
            icr.delmod = HAL_APIC_DELMOD_STARTUP;
            icr.destmod = HAL_APIC_DESTMOD_PHYS;
            icr.level = HAL_APIC_LEVEL_ASSERT;
            icr.shorthand = HAL_APIC_SHORTHAND_NONE;
            icr.trigger_mode = HAL_APIC_TRIGMOD_LEVEL;
            hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
            
            /* According to MP Specification, we should wait for 200us */
            hal_time_delay(1);
        }
    }
    
    return check_lapic();
}
