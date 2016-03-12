// #include "common/include/data.h"
// #include "hal/include/print.h"
// #include "hal/include/apic.h"
// 
// 
// struct ipi_record {
//     int                     registered;
//     int                     vector;
//     hal_interrupt_handler   handler;
// };
// 
// static struct ipi_record ipi_map[256];
// static int cur_ipi_map_index = 0;
// 
// 
// static int general_ipi_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
// {
//     kprintf("General IPI Handler\n");
//     return 0;
// }
// 
// static int get_ipi_vecor(u32 ipi_id)
// {
//     if (ipi_map[ipi_id].registered) {
//         return ipi_map[ipi_id].vector;
//     } else {
//         //panic("IPI not registered!");
//     }
//     
//     return 0;
// }
// 
// 
// void init_ipi()
// {
//     u32 i;
//     
//     for (i = 0; i < sizeof(ipi_map) / sizeof(struct ipi_record); i++) {
//         ipi_map[i].vector = 0;
//         ipi_map[i].handler = NULL;
//         ipi_map[i].registered = 0;
//     }
// }
// 
// /*
//  * Return
//  *      IPI ID
//  */
// int ipi_register(hal_interrupt_handler handler)
// {
//     hal_interrupt_handler real_handler = handler;
//     if (NULL == real_handler) {
//         real_handler = general_ipi_handler;
//     }
//     
//     /* First register a vector */
//     u32 vector = hal_interrupt_vector_alloc(handler);
//     
//     //kprintf("Vector %d\n", vector);
//     
//     /* IPI ID */
//     int ipi_id = cur_ipi_map_index;
//     cur_ipi_map_index++;
//     
//     /* Set the IPI map */
//     ipi_map[ipi_id].vector = (u8)vector;
//     ipi_map[ipi_id].registered = 1;
//     ipi_map[ipi_id].handler = handler;
//     
//     return ipi_id;
// }
// 
// /*
//  * Return
//  *      0 = Failed
//  *      1 = Succeed
//  */
// int ipi_broadcast(int self, int ipi_id)
// {
//     s_hal_apic_interupt_command_register icr;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     icr.delmod = APIC_DELMOD_FIXED;
//     icr.destmod = APIC_DESTMOD_PHYS;
//     icr.level = APIC_LEVEL_ASSERT;
//     icr.shorthand = (self ? APIC_SHORTHAND_ALL_INCL : APIC_SHORTHAND_ALL_EXCL);
//     icr.trigger_mode = APIC_TRIGMOD_EDGE;
//     icr.vector = get_ipi_vecor(ipi_id);
//     
//     lapic_vaddr[APIC_ICR_LO] = icr.value_low;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     if (icr.delivs == APIC_DELIVS_PENDING) {
//         kprintf("IPI is pending.\n");
//     }
//     
//     return check_lapic();
// }
// 
// /*
//  * Return
//  *      0 = Failed
//  *      1 = Succeed
//  */
// int ipi_send(int apicid, int ipi_id)
// {
//     s_hal_apic_interupt_command_register icr;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     icr.delmod = APIC_DELMOD_FIXED;
//     icr.destmod = APIC_DESTMOD_PHYS;
//     icr.level = APIC_LEVEL_ASSERT;
//     icr.shorthand = APIC_SHORTHAND_NONE;
//     icr.trigger_mode = APIC_TRIGMOD_EDGE;
//     icr.vector = get_ipi_vecor(ipi_id);
//     
//     icr.value_high = lapic_vaddr[APIC_ICR_HI];
//     icr.dest = apicid;
//     
//     //kprintf("Target APIC ID %d, Vector %d\n", apicid, lapic_vaddr_ipi_get_vector(ipi_id));
//     
//     lapic_vaddr[APIC_ICR_HI] = icr.value_high;
//     lapic_vaddr[APIC_ICR_LO] = icr.value_low;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     if (icr.delivs == APIC_DELIVS_PENDING) {
//         kprintf("IPI is pending.\n");
//     }
//     
//     return check_lapic();
// }
// 
// /*
//  * Return
//  *      0 = Failed
//  *      1 = Succeed
//  */
// int ipi_self(int ipi_id)
// {
//     s_hal_apic_interupt_command_register icr;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     icr.delmod = APIC_DELMOD_FIXED;
//     icr.destmod = APIC_DESTMOD_PHYS;
//     icr.level = APIC_LEVEL_ASSERT;
//     icr.shorthand = APIC_SHORTHAND_SELF;
//     icr.trigger_mode = APIC_TRIGMOD_EDGE;
//     icr.vector = lapic_vaddr_ipi_get_vector(ipi_id);;
//     
//     lapic_vaddr[APIC_ICR_LO] = icr.value_low;
//     
//     icr.value_low = lapic_vaddr[APIC_ICR_LO];
//     if (icr.delivs == APIC_DELIVS_PENDING) {
//         kprintf("IPI is pending.\n");
//     }
//     
//     return check_lapic();
// }

