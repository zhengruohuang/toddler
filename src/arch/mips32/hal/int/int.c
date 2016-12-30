#include "common/include/data.h"
#include "common/include/task.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"
#include "hal/include/lib.h"
#include "hal/include/cpu.h"
#include "hal/include/task.h"
#include "hal/include/int.h"


void init_int()
{
    u32 ebase = 0;
    u32 sr = 0;
    struct context *ctxt = get_per_cpu(struct context, cur_context);
    u32 stack_top = get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET;
    
//     u32 srsctl = 0;
    
    // Obtain old SR
    __asm__ __volatile__ (
        "mfc0   %0, $12;"
        : "=r" (sr)
        :
    );
    
    // Set BEV to 0 -> use our custom handlers
    sr &= ~0x400000;
    
    // Update SR
    __asm__ __volatile__ (
        "mtc0   %0, $12;"
        "nop;"
        :
        : "r" (sr)
    );
    
    // Obtain the old ebase
    __asm__ __volatile__ (
        "mfc0   %0, $15, 1;"
        : "=r" (ebase)
        :
    );
    
    // Clear and set exception base
    ebase &= ~0x3FFFF000;
    ebase |= ((u32)&int_entry_wrapper_begin) & ~0xFFF;
    
    // Update ebase
    __asm__ __volatile__ (
        "mtc0   %0, $15, 1;"
        "nop;"
        :
        : "r" (ebase)
    );
    
    // Set
    //  k0 - base addr of current context
    //  k1 - kernel stack top
    __asm__ __volatile__ (
        "or $26, %0, $0;"   // k0 ($26) <= ctxt | zero ($0)
        "or $27, %1, $0;"   // k1 ($27) <= stack_top | zero ($0)
        :
        : "r" ((u32)ctxt), "r" (stack_top)
    );
    
//     // QEMU doesn't support shadow register... so we can't use it right now
//     // Obtain old SRSCtl
//     __asm__ __volatile__ (
//         "mfc0   %0, $12, 2;"
//         : "=r" (srsctl)
//         :
//     );
//     
//     // Make sure there is at least 1 set of shadow registers
//     if (!(srsctl & 0x3C000000)) {
//         panic("Need at least 1 set of shadow registers");
//     }
//     
//     // Set ESS to 1
//     srsctl &= ~0xF000;
//     srsctl |= 0x1000;
//     
//     // Update SRSCtl
//     __asm__ __volatile__ (
//         "mtc0   %0, $12, 2;"
//         "nop;"
//         :
//         : "r" (srsctl)
//     );
    
    kprintf("Interrupt base updated, Wrapper @ %x, SR: %x, EBase: %x, Context @ %x, Kernel stack @ %x\n",
            (u32)&int_entry_wrapper_begin, sr, ebase, ctxt, stack_top);
    
//     // Test our handler
//     volatile u32 *bad_addr = (u32 *)0x4096;
//     u32 bad_value = *bad_addr;
//     kprintf("Bad value: %x\n", bad_value);
}

void tlb_refill_handler(struct context *context)
{
    kprintf("TLB Refill!\n");
    while (1);
}

void cache_error_handler(struct context *context)
{
    panic("Toddler doesn't support cache error handling!!\n");
    while (1);
}

void general_except_handler(struct context *context)
{
    kprintf("General exception!\n");
    while (1);
}
