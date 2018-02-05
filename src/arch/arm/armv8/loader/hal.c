#include "common/include/data.h"


void call_hal_entry(ulong entry, ulong stack, ulong bp)
{
//     __asm__ __volatile__ (
//         // Move to System mode
//         "cpsid aif, #0x1f;"
//         
//         // Set up stack top
//         "mov sp, %[stack];"
//         
//         // Pass the argument
//         "mov r0, %[bp];"
//         
//         // Call C
//         "mov pc, %[hal];"
//         :
//         : [stack] "r" (stack), [bp] "r" (bp), [hal] "r" (entry)
//         : "sp", "r0", "memory"
//     );
}
