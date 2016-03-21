#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/acpi.h"
#include "hal/include/int.h"
#include "hal/include/cpu.h"
#include "hal/include/mps.h"
#include "hal/include/apic.h"
#include "hal/include/task.h"
#include "hal/include/time.h"
#include "hal/include/kernel.h"
#include "hal/include/syscall.h"


static void hal_entry()
{
    // First we init the screen then tell the user we are in HAL
    init_video();
    kprintf("We are in HAL!\n");
    
    // Init CPUID
    init_cpuid();
    
    // Init mempool
    init_kalloc();
    
    // Init ACPI
    init_acpi();
    
    // Init MPS
    init_mps();
    
    // Init interrupt
    init_int_handlers();
    init_int_vector();
    
    // Init topo
    init_topo();
    
    // Init APIC
    init_apic();
    
    // Init MP
    init_mp();
    
    // Init TSS
    init_tss();
    
    // Init GDT
    init_gdt();
    
    // Load TSS
    load_tss();
    
    // Init IDT
    init_idt();
    
    // Init interrupt state
    init_int_state();
    
    // Init context
    init_context();
    
    // Init syscall
    init_syscall();
    
    // Init time
    init_rtc();
    init_blocked_delay();
    
    // Init kernel
    init_kmem_zone();
    full_direct_map();
    init_kernel();
    
    //halt();
    
    // Bringup APs
    bringup_mp();
    
    // Start to work
    release_mp_lock();
    start_working();
    
    kprintf("Initialization is done!\n");
}

static void ap_entry()
{
    kprintf("\tSecondary processor is up!\n");
    
    // AP init started
    ap_init_started();
    
    // Init APIC
    init_apic_mp();
    
    // Init TSS
    init_tss_mp();
    
    // Init GDT
    init_gdt_mp();
    
    // Load TSS
    load_tss();
    
    // Init IDT
    load_idt();
    
    // Init interrupt state
    init_int_state_mp();
    
    // Init context
    init_context_mp();
    
    // Init syscall
    init_syscall_mp();
    
    // AP init done
    ap_init_done();
    
    // Start working
    //if (get_cpu_id() == 2) {
        start_working_mp();
    //}
}

static void bios_return()
{
    panic("BIOS invoker not supported!");
}

/*
 * This is the entry point of HAL
 */
void asmlinkage no_opt _start()
{
    struct boot_parameters *boot_param = get_bootparam();
    
    switch (boot_param->hal_start_flag) {
    // Start HAL
    case 0:
        hal_entry();
        break;
        
    // Start AP
    case 1:
        ap_entry();
        break;
        
    // Return from BIOS invoker
    case 2:
        bios_return();
        break;
        
    // Undefined
    default:
        kprintf("ERROR: Undefined HAL start flag: %u\n", boot_param->hal_start_flag);
        halt();
        break;
    }
    
    // Should never reach here
    //panic("Should never reach here!");
    halt();
}
