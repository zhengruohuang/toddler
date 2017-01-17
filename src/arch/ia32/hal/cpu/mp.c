#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "common/include/proc.h"
#include "hal/include/print.h"
#include "hal/include/lib.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/apic.h"


static volatile int ap_bringup_lock = 0;
static volatile int start_working_lock = 1;

ulong tcb_padded_size = 0;
ulong tcb_area_size = 0;
ulong tcb_area_start_vaddr = 0;


/*
 * Per-CPU private data
 */
ulong get_per_cpu_area_start_vaddr(int cpu_id)
{
    assert(cpu_id < num_cpus);
    return PER_CPU_AREA_TOP_VADDR - PER_CPU_AREA_SIZE * (cpu_id + 1);
}

ulong get_my_cpu_area_start_vaddr()
{
    return get_per_cpu_area_start_vaddr(get_cpu_id());
}


/*
 * This will map per-cpu area into the addr space
 */
void init_mp()
{
    kprintf("Initializing multiprocessor support\n");
    
    // Reserve pages
    ulong start_pfn = palloc(PER_CPU_AREA_PAGE_COUNT * num_cpus);
    ulong cur_top_vaddr = PER_CPU_AREA_TOP_VADDR;
    
    // Map per CPU private area
    int i;
    for (i = 0; i < num_cpus; i++) {
        ulong cur_area_vstart = cur_top_vaddr - PER_CPU_AREA_SIZE;

        // Map the per-cpu area
        kernel_indirect_map_array(cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_SIZE, 1, 0);
        
        // Tell the user
        kprintf("\tCPU #%d, per-CPU area: %p -> %p (%dB)\n", i,
            cur_area_vstart, PFN_TO_ADDR(start_pfn), PER_CPU_AREA_SIZE
        );
        
        // Get ready for the next iteration
        cur_top_vaddr -= PER_CPU_AREA_SIZE;
        start_pfn += PER_CPU_AREA_PAGE_COUNT;
    }
}

static void bringup_cpu(int cpu_id)
{
    int apic_id = get_apic_id_by_cpu_id(cpu_id);
    kprintf("Bringing up secondary processor #%d, APIC ID: %d\n", cpu_id, apic_id);
    
    // Set the lock
    ap_bringup_lock = 1;
    
    // Set Loader Function
    u32 *loader_func_type = (u32 *)get_bootparam()->loader_func_type_ptr;
    *loader_func_type = 2;
    
    // HAL start flag
    get_bootparam()->hal_start_flag = 1;
    
    // Page dir
    u32 *page_dir_pfn = (u32 *)get_bootparam()->ap_page_dir_pfn_ptr;
    *page_dir_pfn = KERNEL_PDE_PFN;
    
    // Stack top
    u32 *stack_top = (u32 *)get_bootparam()->ap_stack_top_ptr;
    *stack_top = get_per_cpu_area_start_vaddr(cpu_id) + PER_CPU_STACK_TOP_OFFSET;
    
    // Send IPI
    ipi_send_startup(apic_id);
    
    // Wait until AP is broughtup
    while (ap_bringup_lock) {
        __asm__ __volatile__ ( "pause;" : : );
    }
    
    // Restore loader function and HAL start flag
    *loader_func_type = -1;
    get_bootparam()->hal_start_flag = -1;
    
    kprintf("\tProcessor #%d has been started\n", cpu_id);
}

/*
 * This will send IPI to start all APs
 * This function should be called by BSP
 */
void bringup_mp()
{
    kprintf("Bringing up secondary processors\n");
    
    // Bring up all processors
    int i;
    for (i = 0; i < num_cpus; i++) {
        if (i == get_cpu_id()) {
            continue;
        }
        
        bringup_cpu(i);
    }
    
    kprintf("All processors have been successfully brought up\n");
}

/*
 * This will allow all APs start working
 */
void release_mp_lock()
{
    start_working_lock = 0;
    __sync_synchronize();
}

/*
 * Secondary processor initializaton started
 */
void ap_init_started()
{
}

/*
 * Secondary processor initializaton done
 */
void ap_init_done()
{
    kprintf("\tSecondary processor initialzied, waiting for the MP lock release\n");
    ap_bringup_lock = 0;
    
    while (start_working_lock) {
        __asm__ __volatile__ ( "pause;" : : );
    }
}
