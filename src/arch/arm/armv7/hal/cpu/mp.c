#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/memlayout.h"
#include "common/include/proc.h"
#include "common/include/reg.h"
#include "common/include/atomic.h"
#include "hal/include/print.h"
#include "hal/include/string.h"
#include "hal/include/bootparam.h"
#include "hal/include/debug.h"
#include "hal/include/mem.h"
#include "hal/include/cpu.h"
#include "hal/include/periph.h"


static volatile ulong ap_bringup_lock = 0;
static volatile ulong start_working_lock = 1;

static ulong per_cpu_area_start_paddr = 0;
static ulong per_cpu_area_start_vaddr = 0;

ulong tcb_padded_size = 0;
ulong tcb_area_start_vaddr = 0;
ulong tcb_area_size = 0;


/*
 * Get my CPU id
 */
int get_cpu_id()
{
    u32 cpu_num = 0;
    
    read_cpu_id(cpu_num);
    cpu_num &= 0x3;
    
    assert(cpu_num < num_cpus);
    
    return (int)cpu_num;
}


/*
 * Per-CPU private data
 */
ulong get_per_cpu_area_start_paddr(int cpu_id)
{
    assert(cpu_id < num_cpus);
    return per_cpu_area_start_paddr + PER_CPU_AREA_SIZE * cpu_id;
}

ulong get_my_cpu_area_start_paddr()
{
    return get_per_cpu_area_start_paddr(get_cpu_id());
}

ulong get_per_cpu_area_start_vaddr(int cpu_id)
{
    assert(cpu_id < num_cpus);
    return per_cpu_area_start_vaddr + PER_CPU_AREA_SIZE * cpu_id;
}

ulong get_my_cpu_area_start_vaddr()
{
    return get_per_cpu_area_start_vaddr(get_cpu_id());
}


/*
 * Init
 */
void init_mp()
{
    int i;
    
    kprintf("Initializing multiprocessor support\n");
    
    // Reserve pages
    ulong per_cpu_are_start_pfn = palloc(PER_CPU_AREA_PAGE_COUNT * num_cpus);
    per_cpu_area_start_paddr = PFN_TO_ADDR(per_cpu_are_start_pfn);
    per_cpu_area_start_vaddr = PER_CPU_AREA_BASE_VADDR;
    
    // Map per CPU private area
    ulong cur_area_pstart = PFN_TO_ADDR(per_cpu_are_start_pfn);
    ulong cur_area_vstart = per_cpu_area_start_vaddr;
    
    kprintf("\tPer-CPU area start phys @ %p, virt @ %p\n",
            (void *)cur_area_pstart, (void *)cur_area_vstart);
    
    for (i = 0; i < num_cpus; i++) {
        // Map the page to its new virt location
        kernel_map_per_cpu_area(cur_area_vstart, cur_area_pstart, PER_CPU_AREA_SIZE);
        
        // Zero the area
        memzero((void *)cur_area_vstart, PER_CPU_AREA_SIZE);
        
        // Tell the user
        kprintf("\tCPU #%d, per-CPU area: %p -> %p (%d bytes)\n", i,
            (void *)cur_area_vstart, (void *)cur_area_pstart, PER_CPU_AREA_SIZE
        );
        
        cur_area_pstart += PER_CPU_AREA_SIZE;
        cur_area_vstart += PER_CPU_AREA_SIZE;
    }
    
    kprintf("MP support init done!\n");
}


/*
 * MP bringing-up
 */
static void bringup_cpu(int cpu_id)
{
    struct boot_parameters *bp = get_bootparam();
    
    kprintf("Bringing up secondary processor #%d\n", cpu_id);
    
    // Set the lock
    atomic_write(&ap_bringup_lock, 1);
    
    // Set Loader Function
    bp->loader_func_type = 2;
    
    // HAL start flag
    get_bootparam()->hal_start_flag = 1;
    
    // Stack top
    bp->ap_stack_top = get_per_cpu_area_start_vaddr(cpu_id) + PER_CPU_STACK_TOP_OFFSET - 16;
    
    // Send IPI
    periph_waekup_cpu(cpu_id, bp->ap_entry);
    
    // Wait until AP is broughtup
    while (ap_bringup_lock) {
        atomic_membar();
    }
    
    // Restore loader function and HAL start flag
    bp->loader_func_type = 1;
    bp->hal_start_flag = -1;
    
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
    atomic_membar();
//     __sync_synchronize();
}

/*
 * Secondary processor initializaton done
 */
void ap_init_done()
{
    kprintf("\tSecondary processor initialzied, waiting for the MP lock release\n");
    ap_bringup_lock = 0;
    
    while (start_working_lock) {
        atomic_membar();
    }
}
