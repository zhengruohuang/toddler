#ifndef __ARCH_IA32_COMMON_INCLUDE_CONTEXT__
#define __ARCH_IA32_COMMON_INCLUDE_CONTEXT__


#include "common/include/data.h"


/*
 * Context
 */
struct context {
    // pushad/popad
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp; // This field has no use, and POPAD will ignore it. It is PUSHAD that saves it automatically
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    
    // Pushed by HW upon an interrupt
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
};


#endif
