// #include "common/include/data.h"
// #include "hal/include/print.h"
// #include "hal/include/apic.h"
// 
// 
// static int lapic_error_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
// {
//     return 0;
// }
// 
// static int lapic_spurious_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
// {
//     return 0;
// }
// 
// static int lapic_handler_start_to_work(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
// {
//     hal_init_start_to_work_mp();
//     
//     /*
//      * Send a start message to kernel
//      */
//     s_message *msg = hal_ipc_create_message();
//     
//     msg->sender_task_id = 0;
//     msg->sender_worker_id = 0;
//     
//     msg->receiver_task_id = 0;
//     msg->receiver_worker_id = 0;
//     
//     msg->type = message_type_start;
//     
//     /* Start APIC */
//     hal_apic_start();
//     
//     hal_apic_lapic_eoi();
//     
//     return 0;
// }
// 
// static int lapic_handler_bringup(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
// {
//     /*
//      * Let local APIC start to work
//      */
//     hal_init_start_to_work_mp();
//     
//     /*
//      * Note that to get APs working, we need to do an EOI here, otherwise
//      * APs would no longer receive interrupts
//      */
//     hal_apic_lapic_eoi();
//     
//     //kprintf("Started!");
//     
//     return 0;
// }
// 
// void init_lapic_mp()
// {
//     /* Initialize Local APIC */
//     u64 msr = 0;
//     hal_cpu_msr_read(0x1b, &msr);
//     
//     if (!(msr & (1 << 11))) {
//         /*MSR_IA32_APICBASE_ENABLE | APIC_DEFAULT_PHYS_BASE*/
//         msr |= (1 << 11) | 0xfee00000;
//         
//         kprintf("Local APIC reenabled\n");
//         
//         hal_cpu_msr_write(0x1b, &msr);
//     }
//     
//     /* Initialize LVT Error register. */
//     s_hal_apic_lvt_error_register error;
//     
//     error.value = hal_apic_lapic[HAL_APIC_LVT_ERR];
//     error.masked = 1;
//     error.vector = hal_interrupt_vector_alloc(lapic_error_handler);
//     hal_apic_lapic[HAL_APIC_LVT_ERR] = error.value;
//     
//     /* Initialize LVT LINT0 register. */
//     s_hal_apic_lvt_lint_register lint;
//     
//     lint.value = hal_apic_lapic[HAL_APIC_LVT_LINT0];
//     lint.masked = 1;
//     hal_apic_lapic[HAL_APIC_LVT_LINT0] = lint.value;
//     
//     /* Initialize LVT LINT1 register. */
//     lint.value = hal_apic_lapic[HAL_APIC_LVT_LINT1];
//     lint.masked = 1;
//     hal_apic_lapic[HAL_APIC_LVT_LINT1] = lint.value;
//     
//     /* Task Priority Register initialization. */
//     s_hal_apic_task_priority_register tpr;
//     
//     tpr.value = hal_apic_lapic[HAL_APIC_TPR];
//     tpr.pri_sc = 0;
//     tpr.pri = 0;
//     hal_apic_lapic[HAL_APIC_TPR] = tpr.value;
//     
//     /* Spurious-Interrupt Vector Register initialization. */
//     s_hal_apic_spurious_vector_register svr;
//     
//     svr.value = hal_apic_lapic[HAL_APIC_SVR];
//     svr.vector = hal_interrupt_vector_alloc(lapic_spurious_handler);
//     svr.lapic_enabled = 1;
//     svr.focus_checking = 1;
//     hal_apic_lapic[HAL_APIC_SVR] = svr.value;
//     
//     //if (CPU->arch.family >= 6)
//     //enable_lapic_in_msr();
//     
//     /* Interrupt Command Register initialization. */
//     //s_hal_apic_interupt_command_register icr;
//     
//     //icr.value_low = hal_apic_lapic[HAL_APIC_ICR_LO];
//     //icr.delmod = HAL_APIC_DELMOD_INIT;
//     //icr.destmod = HAL_APIC_DESTMOD_PHYS;
//     //icr.level = HAL_APIC_LEVEL_DEASSERT;
//     //icr.shorthand = HAL_APIC_SHORTHAND_ALL_INCL;
//     //icr.trigger_mode = HAL_APIC_TRIGMOD_LEVEL;
//     //hal_apic_lapic[HAL_APIC_ICR_LO] = icr.value_low;
//     
//     /* Initialize Logical Destination Register, though we actually do not use it. */
//     s_hal_apic_logical_destination_register ldr;
//     
//     ldr.value = hal_apic_lapic[HAL_APIC_LDR];
//     ldr.id = hal_mp_ap_startup_current_processor_id;
//     hal_apic_lapic[HAL_APIC_LDR] = ldr.value;
//     
//     /* Initialize Destination Format Register as Flat mode. */
//     s_hal_apic_destination_format_register dfr;
//     
//     dfr.value = hal_apic_lapic[HAL_APIC_DFR];
//     dfr.model = HAL_APIC_MODEL_FLAT;
//     hal_apic_lapic[HAL_APIC_DFR] = dfr.value;
// }
// 
// void start_bsp_lapic()
// {
//     /* Register the start-to-work IPI */
//     u32 kernel_ipi_id = hal_apic_lapic_ipi_register(lapic_handler_start_to_work);
//     
//     /* Send start-to-work IPI */
//     u32 i;
//     for (i = 0; i < hal_cpu_topology_machine.logical_processor_count; i++) {
//         if (
//             hal_cpu_all_logical_processors[i].is_bootstrap &&
//             hal_cpu_all_logical_processors[i].usable
//         ) {
//             hal_apic_lapic_ipi_send(
//                 hal_cpu_all_logical_processors[i].processor_id,
//                 kernel_ipi_id
//             );
//             
//             break;
//         }
//     }
//     
//     /* Wait for 10 milliseconds */
//     hal_time_delay(10);
// }
// 
// void start_lapic()
// {
//     /* Register the start-to-work IPI */
//     u32 start_ipi_id = hal_apic_lapic_ipi_register(lapic_handler_bringup);
//     
//     /* Enable interrupts in all APs */
//     hal_mp_ap_enable_interrupts = 1;
//     
//     /* Wait until all the APs have enabled interrupts */
//     u32 loop_count_limit = 10;
//     u32 current_loop_count = 0;
//     do {
//         if (hal_mp_ap_startup_interrupts_enabled_count == hal_cpu_topology_machine.logical_processor_count - 1) {
//             break;
//         }
//         
//         /* If there is bug that not all APs started successfully */
//         if (current_loop_count >= loop_count_limit) {
//             CRASH("Not All APs Started Successfully!");
//         }
//         
//         /* Wait for 100 milliseconds */
//         hal_time_delay(100);
//         current_loop_count++;
//     } while (1);
//     hal_time_delay(10);
//     
//     /* Send start-to-work IPI */
//     u32 i;
//     for (i = 0; i < hal_cpu_topology_machine.logical_processor_count; i++) {
//         if (
//             !hal_cpu_all_logical_processors[i].is_bootstrap &&
//             hal_cpu_all_logical_processors[i].usable
//         ) {
//             hal_apic_lapic_ipi_send(
//                 hal_cpu_all_logical_processors[i].processor_id,
//                 start_ipi_id
//             );
//             
//             /* Wait for 10 milliseconds */
//             hal_time_delay(10);
//         }
//     }
//     
//     
//     //hal_apic_lapic_ipi_send(4, start_ipi_id);
//     
//     /* Broadcast a start-to-work IPI */
//     //hal_apic_lapic_ipi_broadcast(0, start_ipi_id);
// }
// 
// void lapic_eoi()
// {
//     hal_apic_lapic[HAL_APIC_EOI] = 0;
// }
// 
// int get_lapic_id()
// {
//     s_hal_apic_lapic_id idreg;
//     
//     idreg.value = hal_apic_lapic[HAL_APIC_LAPIC_ID];
//     
//     return idreg.apic_id;
// }
// 
// int check_lapic()
// {
//     s_hal_apic_error_status_register esr;
//     
//     esr.value = hal_apic_lapic[HAL_APIC_ESR];
//     
//     if (esr.send_checksum_error) {
//         kprintf("Send Checksum Error\n");
//     }
//     
//     if (esr.receive_checksum_error) {
//         kprintf("Receive Checksum Error\n");
//     }
//     
//     if (esr.send_accept_error) {
//         kprintf("Send Accept Error\n");
//     }
//     
//     if (esr.receive_accept_error) {
//         kprintf("Receive Accept Error\n");
//     }
//     
//     if (esr.send_illegal_vector) {
//         kprintf("Send Illegal Vector\n");
//     }
//     
//     if (esr.received_illegal_vector) {
//         kprintf("Received Illegal Vector\n");
//     }
//     
//     if (esr.illegal_register_address) {
//         kprintf("Illegal Register Address\n");
//     }
//     
//     return !esr.err_bitmap;
// }
