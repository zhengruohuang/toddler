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
//     //Start APIC */
//     hal_apic_start();
//     
//     lapic_vaddr_eoi();
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
//     lapic_vaddr_eoi();
//     
//     //kprintf("Started!");
//     
//     return 0;
// }
// 
// 
// void start_bsp_lapic()
// {
//     //Register the start-to-work IPI */
//     u32 kernel_ipi_id = lapic_vaddr_ipi_register(lapic_handler_start_to_work);
//     
//     //Send start-to-work IPI */
//     u32 i;
//     for (i = 0; i < hal_cpu_topology_machine.logical_processor_count; i++) {
//         if (
//             hal_cpu_all_logical_processors[i].is_bootstrap &&
//             hal_cpu_all_logical_processors[i].usable
//         ) {
//             lapic_vaddr_ipi_send(
//                 hal_cpu_all_logical_processors[i].processor_id,
//                 kernel_ipi_id
//             );
//             
//             break;
//         }
//     }
//     
//     //Wait for 10 milliseconds */
//     hal_time_delay(10);
// }
// 
// void start_lapic()
// {
//     //Register the start-to-work IPI */
//     u32 start_ipi_id = lapic_vaddr_ipi_register(lapic_handler_bringup);
//     
//     //Enable interrupts in all APs */
//     hal_mp_ap_enable_interrupts = 1;
//     
//     //Wait until all the APs have enabled interrupts */
//     u32 loop_count_limit = 10;
//     u32 current_loop_count = 0;
//     do {
//         if (hal_mp_ap_startup_interrupts_enabled_count == hal_cpu_topology_machine.logical_processor_count - 1) {
//             break;
//         }
//         
//         //If there is bug that not all APs started successfully */
//         if (current_loop_count >= loop_count_limit) {
//             CRASH("Not All APs Started Successfully!");
//         }
//         
//         //Wait for 100 milliseconds */
//         hal_time_delay(100);
//         current_loop_count++;
//     } while (1);
//     hal_time_delay(10);
//     
//     //Send start-to-work IPI */
//     u32 i;
//     for (i = 0; i < hal_cpu_topology_machine.logical_processor_count; i++) {
//         if (
//             !hal_cpu_all_logical_processors[i].is_bootstrap &&
//             hal_cpu_all_logical_processors[i].usable
//         ) {
//             lapic_vaddr_ipi_send(
//                 hal_cpu_all_logical_processors[i].processor_id,
//                 start_ipi_id
//             );
//             
//             //Wait for 10 milliseconds */
//             hal_time_delay(10);
//         }
//     }
//     
//     
//     //lapic_vaddr_ipi_send(4, start_ipi_id);
//     
//     //Broadcast a start-to-work IPI */
//     //lapic_vaddr_ipi_broadcast(0, start_ipi_id);
// }
// 


#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"
#include "hal/include/mps.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/int.h"
#include "hal/include/time.h"
#include "hal/include/apic.h"


ulong lapic_paddr = LAPIC_DEFAULT_PADDR;
static volatile u32 *lapic_vaddr = (u32 *)LAPIC_VADDR;


// Records the APCI ID of each processor
int bsp_apic_id = -1;
ulong *apic_id_map = NULL;
int cur_apic_id_map_index = 0;


/*
 * Interrupt handlers
 */
 int lapic_error_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    return 0;
}

 int lapic_spurious_handler(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    return 0;
}

 int check_lapic()
{
    struct apic_error_status_register esr;
    
    esr.value = lapic_vaddr[APIC_ESR];
    
    if (esr.send_checksum_error) {
        kprintf("Send Checksum Error\n");
    }
    
    if (esr.receive_checksum_error) {
        kprintf("Receive Checksum Error\n");
    }
    
    if (esr.send_accept_error) {
        kprintf("Send Accept Error\n");
    }
    
    if (esr.receive_accept_error) {
        kprintf("Receive Accept Error\n");
    }
    
    if (esr.send_illegal_vector) {
        kprintf("Send Illegal Vector\n");
    }
    
    if (esr.received_illegal_vector) {
        kprintf("Received Illegal Vector\n");
    }
    
    if (esr.illegal_register_address) {
        kprintf("Illegal Register Address\n");
    }
    
    return !esr.err_bitmap;
}

void lapic_eoi()
{
    lapic_vaddr[APIC_EOI] = 0;
}

int get_apic_id_by_cpu_id(int cpu_id)
{
    assert(cpu_id < num_cpus);
    
    return apic_id_map[cpu_id];
}

int get_cpu_id_by_apic_id(int apic_id)
{
    int i;
    
    for (i = 0; i < num_cpus; i++) {
        if (apic_id_map[i] == apic_id) {
            return i;
        }
    }
    
    panic("Unable to find APIC ID: %d", apic_id);
    
    return -1;
}

int get_apic_id()
{
    struct apic_lapic_id idreg;
    
    idreg.value = lapic_vaddr[APIC_LAPIC_ID];
    
    return idreg.apic_id;
}

int get_cpu_id()
{
    if (apic_supported) {
        return get_cpu_id_by_apic_id(get_apic_id());
    } else {
        return 0;
    }
}

void init_lapic_mp()
{
    // Insert into APIC ID map
//     int cur_cpu_index = cur_apic_id_map_index++;
//     int cur_apic_id = get_apic_id();
//     apic_id_map[cur_cpu_index] = cur_apic_id;
//     
//     // BSP's APIC ID
//     if (0 == cur_cpu_index) {
//         bsp_apic_id = cur_apic_id;
//     }
    
    kprintf("\tSetting up local APIC\n");
    
    //Initialize Local APIC */
    u64 msr = 0;
    msr_read(0x1b, &msr);
    
    if (!(msr & (1 << 11))) {
        /*MSR_IA32_APICBASE_ENABLE | APIC_DEFAULT_PHYS_BASE*/
        msr |= (1 << 11) | lapic_paddr;
        
        kprintf("Local APIC reenabled\n");
        
        msr_write(0x1b, &msr);
    }
    
    //Initialize LVT Error register. */
    struct apic_lvt_error_register error;
    
    error.value = lapic_vaddr[APIC_LVT_ERR];
    error.masked = 1;
    error.vector = alloc_int_vector(lapic_error_handler);
    lapic_vaddr[APIC_LVT_ERR] = error.value;
    
    //Initialize LVT LINT0 register. */
    struct apic_lvt_lint_register lint;
    
    lint.value = lapic_vaddr[APIC_LVT_LINT0];
    lint.masked = 1;
    lapic_vaddr[APIC_LVT_LINT0] = lint.value;
    
    //Initialize LVT LINT1 register. */
    lint.value = lapic_vaddr[APIC_LVT_LINT1];
    lint.masked = 1;
    lapic_vaddr[APIC_LVT_LINT1] = lint.value;
    
    //Task Priority Register initialization. */
    struct apic_task_priority_register tpr;
    
    tpr.value = lapic_vaddr[APIC_TPR];
    tpr.pri_sc = 0;
    tpr.pri = 0;
    lapic_vaddr[APIC_TPR] = tpr.value;
    
    //Spurious-Interrupt Vector Register initialization. */
    struct apic_spurious_vector_register svr;
    
    svr.value = lapic_vaddr[APIC_SVR];
    svr.vector = alloc_int_vector(lapic_spurious_handler);
    svr.lapic_enabled = 1;
    svr.focus_checking = 1;
    lapic_vaddr[APIC_SVR] = svr.value;
    
    //if (CPU->arch.family >= 6)
    //enable_lapic_in_msr();
    
    //Interrupt Command Register initialization. */
    //struct apic_interupt_command_register icr;
    
    //icr.value_low = lapic_vaddr[APIC_ICR_LO];
    //icr.delmod = APIC_DELMOD_INIT;
    //icr.destmod = APIC_DESTMOD_PHYS;
    //icr.level = APIC_LEVEL_DEASSERT;
    //icr.shorthand = APIC_SHORTHAND_ALL_INCL;
    //icr.trigger_mode = APIC_TRIGMOD_LEVEL;
    //lapic_vaddr[APIC_ICR_LO] = icr.value_low;
    
    //Initialize Logical Destination Register, though we actually do not use it. */
    struct apic_logical_destination_register ldr;
    
    ldr.value = lapic_vaddr[APIC_LDR];
    ldr.id = get_cpu_id_by_apic_id(get_apic_id());
    lapic_vaddr[APIC_LDR] = ldr.value;
    
    //Initialize Destination Format Register as Flat mode. */
    struct apic_destination_format_register dfr;
    
    dfr.value = lapic_vaddr[APIC_DFR];
    dfr.model = APIC_MODEL_FLAT;
    lapic_vaddr[APIC_DFR] = dfr.value;
}

void init_lapic()
{
    // Figure out LAPIC paddr
    if (acpi_supported && madt_supported) {
        if (madt_lapic_addr) {
            lapic_paddr = madt_lapic_addr;
        }
    } else if (mps_supported) {
        if (mps_lapic_addr) {
            lapic_paddr = mps_lapic_addr;
        }
    } else {
        panic("APIC Init: Unable to figure out LAPIC paddr!");
    }
    
    // Map LAPIC
    kernel_indirect_map_array((ulong)lapic_vaddr, lapic_paddr, PAGE_SIZE, 1, 1);
    get_bootparam()->hal_vspace_end -= PAGE_SIZE;
    kprintf("\tLocal APIC mapped: %p -> %p\n", lapic_vaddr, lapic_paddr);
    
    // Get BSP's APIC ID
    bsp_apic_id = get_apic_id();
    
    // Alloc APICID map
    int i;
    struct acpi_madt_lapic *entry = NULL;
    apic_id_map = kalloc(sizeof(ulong) * num_cpus);
    for (i = 0; i < num_cpus; i++) {
        int usable = -1;
        int apic_id = -1;
        
        if (acpi_supported && madt_supported) {
            entry = get_next_acpi_lapic_entry(entry, &usable);
            assert(entry);
            apic_id = entry->apic_id;
        }
        
        assert(apic_id != -1);
        kprintf("\t APIC ID added: %d, index: %d\n", apic_id, i);
        apic_id_map[i] = apic_id;
    }
    
    // Move BSP to the first entry
    for (i = 0; i < num_cpus; i++) {
        if (i && apic_id_map[i] == bsp_apic_id) {
            apic_id_map[i] = apic_id_map[0];
            apic_id_map[0] = bsp_apic_id;
            break;
        }
    }
    
    // Init LAPIC on BSP
    init_lapic_mp();
}

/*
 * Return
 *      0 = Failed
 *      1 = Succeed
 */
int ipi_send_startup(int apicid)
{
    volatile struct apic_interupt_command_register icr;
    
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
