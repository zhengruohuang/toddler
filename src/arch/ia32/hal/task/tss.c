#include "common/include/data.h"
#include "common/include/memlayout.h"
#include "hal/include/print.h"
#include "hal/include/cpu.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/task.h"


static dec_per_cpu(struct tss, tss_user);
dec_per_cpu(u32, tss_user_base);
dec_per_cpu(u32, tss_user_limit);

static dec_per_cpu(struct tss, tss_iopb);
dec_per_cpu(u32, tss_iopb_base);
dec_per_cpu(u32, tss_iopb_limit);


void load_tss()
{
    u32 selector = GDT_SELECTOR_TSS_USER;

    kprintf("\tLoading TSS, selector: %x ... ", selector);
    
    __asm__ __volatile__
    (
        "ltr    %%ax;"
        :
        : "a" (selector)
    );
    
    kprintf("Done\n");
}

static void construct_tss()
{
    struct tss *cur_user = get_per_cpu(struct tss, tss_user);
    u32 *cur_user_base = get_per_cpu(u32, tss_user_base);
    u32 *cur_user_limit = get_per_cpu(u32, tss_user_limit);
    
    struct tss *cur_iopb = get_per_cpu(struct tss, tss_iopb);
    u32 *cur_iopb_base = get_per_cpu(u32, tss_iopb_base);
    u32 *cur_iopb_limit = get_per_cpu(u32, tss_iopb_limit);
    
    // Zero the memory
    memzero(cur_user, sizeof(struct tss));
    memzero(cur_iopb, sizeof(struct tss));
    
    // Set ss and esp
    cur_iopb->ss0 = GDT_SELECTOR_DATA_K;
    cur_iopb->esp0 = get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET;
    
    cur_user->ss0 = GDT_SELECTOR_DATA_K;
    cur_user->esp0 = get_my_cpu_area_start_vaddr() + PER_CPU_STACK_TOP_OFFSET;
    
    // Set IOPB
    cur_user->iopb_addr = 0;
    cur_iopb->iopb_addr = 0;
    
    // Set base address and limit
    *cur_user_base = (u32)cur_user;
    *cur_user_limit = sizeof(struct tss) - 1;
    
    *cur_iopb_base = (u32)cur_iopb;
    *cur_iopb_limit = sizeof(struct tss) - 1;
}

void init_tss_mp()
{
    kprintf("\tConstructing TSS\n");
    construct_tss();
}

void init_tss()
{
    kprintf("\tConstructing TSS\n");
    construct_tss();
}
