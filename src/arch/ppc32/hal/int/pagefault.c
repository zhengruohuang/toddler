#include "common/include/data.h"
#include "common/include/kdisp.h"
#include "common/include/context.h"
#include "common/include/memory.h"
#include "hal/include/print.h"
#include "hal/include/debug.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/percpu.h"
#include "hal/include/vector.h"
#include "hal/include/int.h"
#include "hal/include/vecnum.h"


static int isi_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong asid = *get_per_cpu(ulong, cur_asid);
    ulong vaddr = ALIGN_DOWN((ulong)context->context->pc, PAGE_SIZE);
    
    ulong page_dir_pfn = *get_per_cpu(ulong, cur_page_dir_pfn);
    ulong paddr = get_paddr(page_dir_pfn, vaddr);
    assert(paddr);
    
//     kprintf("To handle isi, vaddr @ %p, paddr @ %p, asid: %p\n",
//         (void *)vaddr, (void *)paddr, (void *)asid
//     );
    fill_pht_by_addr(asid, vaddr, paddr, PAGE_SIZE, 0, KERNEL_PHT_REGULAR);
    
    return INT_HANDLE_TYPE_TAKEOVER;
}

static int dsi_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong asid = *get_per_cpu(ulong, cur_asid);
    ulong vaddr = 0;
    ulong cause = 0;
    
    __asm__ __volatile__
    (
        "mfspr %[vaddr], 19;"
        "mfspr %[cause], 18;"
        : [vaddr]"=r"(vaddr),  [cause]"=r"(cause)
        :
    );
    vaddr = ALIGN_DOWN(vaddr, PAGE_SIZE);
    
    ulong page_dir_pfn = *get_per_cpu(ulong, cur_page_dir_pfn);
    ulong paddr = get_paddr(page_dir_pfn, vaddr);
    
    if (!paddr) {
        kprintf("DSI vaddr @ %p, page dir pfn @ %p, cause: %p, asid: %p, pc: %p, r3: %p, r13: %p\n",
                (void *)vaddr, (void *)page_dir_pfn, (void *)cause, (void *)asid,
                (void *)(ulong)context->context->pc,
                (void *)(ulong)context->context->r3, (void *)(ulong)context->context->r13);
    }
    assert(paddr);
    
//     kprintf("To handle dsi, pc @ %p, vaddr @ %p, paddr @ %p, asid: %p\n",
//         (void *)(ulong)context->context->pc, (void *)vaddr, (void *)paddr, (void *)asid
//     );
    fill_pht_by_addr(asid, vaddr, paddr, PAGE_SIZE, 0, KERNEL_PHT_REGULAR);
    
    return INT_HANDLE_TYPE_TAKEOVER;
}

static int illegal_instr_handler(struct int_context *context, struct kernel_dispatch_info *kdi)
{
    ulong asid = *get_per_cpu(ulong, cur_asid);
    ulong pc = (ulong)context->context->pc;
    
    ulong page_dir_pfn = *get_per_cpu(ulong, cur_page_dir_pfn);
    ulong paddr = get_paddr(page_dir_pfn, pc);
    
    kprintf("Illegal instr @ %p, paddr @ %p, asid: %p, srr1: %p\n",
        (void *)pc, (void *)paddr, (void *)asid, (void *)(ulong)context->context->msr
    );
    
    while (1);
}


void init_pagefault()
{
    set_int_vector(INT_VECTOR_ISI, isi_handler);
    set_int_vector(INT_VECTOR_DSI, dsi_handler);
    set_int_vector(INT_VECTOR_PROGRAM, illegal_instr_handler);
}
