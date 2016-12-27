#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/int.h"


void init_int()
{
    u32 ebase = 0;
    u32 sr = 0;
    
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
    ebase |= ((u32)&tlb_refill_entry) & ~0xFFF;
    
    // Update ebase
    __asm__ __volatile__ (
        "mtc0   %0, $15, 1;"
        "nop;"
        :
        : "r" (ebase)
    );
    
    kprintf("Interrupt base updated, TLB refill entry @ %x, SR: %x, EBase: %x\n", (u32)&tlb_refill_entry, sr, ebase);
    
//     // Test our handler
//     volatile u32 *bad_addr = (u32 *)0x4096;
//     u32 bad_value = *bad_addr;
//     kprintf("Bad value: %x\n", bad_value);
}

void tlb_refill_handler()
{
    kprintf("TLB Refill!\n");
    while (1);
}

void cache_error_handler()
{
    kprintf("Cache error!\n");
    while (1);
}

void general_except_handler()
{
    kprintf("General exception!\n");
    while (1);
}
