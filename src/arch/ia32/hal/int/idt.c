#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/int.h"


static struct idt int_idt;


static void idt_entry(ulong vector, ulong desc_type, void *handler, ulong privilege)
{
    struct idt_gate *p_gate = (struct idt_gate*)(&int_idt.entries[vector]);
    ulong base = (ulong)handler;
    p_gate->offset_low      = base & 0xFFFF;
    p_gate->selector        = GDT_SELECTOR_CODE_K;
    p_gate->dcount          = 0;
    p_gate->attr            = desc_type | (privilege << 5);
    p_gate->offset_high     = (base >> 16) & 0xFFFF;
}

static void load_idt()
{
    __asm__ __volatile__
    (
        "lidt   (%%ebx);"
        :
        : "b" (&(int_idt.idtr_value))
    );
}

void init_idt_mp()
{
    load_idt();
    // Load IDT
    __asm__ __volatile__
    (
        "lidt   (%%ebx);"
        :
        : "b" (&int_idt.idtr_value)
    );
}

void init_idt()
{
    // All of the Traps and Faults are initialized as Interupt Gates
    kprintf("Initializing interrupt descriptor table\n");
    
    // Exceptions and Traps
    kprintf("\tConstructing IDT entries\n");
    idt_entry(VEC_DIVIDE,           DA_IntGate, int_handler_divide,                 PRIVILEGE_KRNL);
    idt_entry(VEC_DEBUG,            DA_IntGate, int_handler_debug,                  PRIVILEGE_KRNL);
    idt_entry(VEC_NMI,              DA_IntGate, int_handler_nmi,                    PRIVILEGE_KRNL);
    idt_entry(VEC_BREAKPOINT,       DA_IntGate, int_handler_breakpoint,             PRIVILEGE_USER);
    idt_entry(VEC_OVERFLOW,         DA_IntGate, int_handler_overflow,               PRIVILEGE_USER);
    idt_entry(VEC_BOUNDS,           DA_IntGate, int_handler_bounds_check,           PRIVILEGE_KRNL);
    idt_entry(VEC_INVAL_OP,         DA_IntGate, int_handler_invalid_opcode,         PRIVILEGE_KRNL);
    idt_entry(VEC_COPROC_NOT,       DA_IntGate, int_handler_copr_not_available,     PRIVILEGE_KRNL);
    idt_entry(VEC_DOUBLE_FAULT,     DA_IntGate, int_handler_double_fault,           PRIVILEGE_KRNL);
    idt_entry(VEC_COPROC_SEG,       DA_IntGate, int_handler_copr_seg_overrun,       PRIVILEGE_KRNL);
    idt_entry(VEC_INVAL_TSS,        DA_IntGate, int_handler_invalid_tss,            PRIVILEGE_KRNL);
    idt_entry(VEC_SEG_NOT,          DA_IntGate, int_handler_segment_not_present,    PRIVILEGE_KRNL);
    idt_entry(VEC_STACK_FAULT,      DA_IntGate, int_handler_stack_exception,        PRIVILEGE_KRNL);
    idt_entry(VEC_PROTECTION,       DA_IntGate, int_handler_general_protection,     PRIVILEGE_KRNL);
    idt_entry(VEC_PAGE_FAULT,       DA_IntGate, int_handler_page_fault,             PRIVILEGE_KRNL);
    idt_entry(VEC_COPROC_ERR,       DA_IntGate, int_handler_copr_error,             PRIVILEGE_KRNL);
    idt_entry(VEC_ALIGN_CHECK,      DA_IntGate, int_handler_align_check,            PRIVILEGE_KRNL);
    idt_entry(VEC_MACHINE_CHECK,    DA_IntGate, int_handler_machine_check,          PRIVILEGE_KRNL);
    idt_entry(VEC_SIMD,             DA_IntGate, int_handler_simd_error,             PRIVILEGE_KRNL);
    
    // Generate Handlers
    kprintf("\tSetting up interrupt handlers\n");
    u8 *temp = (u8 *)int_handler_template_begin;
    ulong template_size = (ulong)int_handler_template_end - (ulong)int_handler_template_begin;
    int vec_num_offset = 0;
    
    int i;
    for (i = 0; i <= template_size - 4; i++) {
        if (
            0x32 == temp[i + 0] &&
            0x76 == temp[i + 1] &&
            0xba == temp[i + 2] &&
            0xfe == temp[i + 3]
        ) {
            vec_num_offset = i;
            break;
        }
    }
    
    kprintf("\tTemplate Begin %p, End %p, Size %d\n",
            int_handler_template_begin, int_handler_template_end,
            (int)template_size
    );
    kprintf("\tVector Number Offset %d\n", vec_num_offset);
    
    if (!vec_num_offset) {
        panic("Unable to initialize interrupt handlers!");
    }
    
    kprintf("\tSetting handler vector and IDT entries ");
    ulong cur_hdlr_start = (ulong)int_handlers;
    u32 cur_vec = 0;
    for (i = 0; i < IDT_ENTRY_COUNT - VEC_IRQ_START; i++) {
        cur_hdlr_start = (u32)int_handlers + template_size * i;
        cur_vec = VEC_IRQ_START + i;
        //kprintf("\t\tHandler #%d, Start %h ...", cur_vec, cur_hdlr_start);
        kprintf(".");
        
        // Set interrupt vector
        //volatile u32 set_vec_num_before = *((u32 *)(cur_hdlr_start + vec_num_offset));
        *((u32 *)(cur_hdlr_start + vec_num_offset)) = cur_vec;
        //volatile u32 set_vec_num_after = *((u32*)(cur_hdlr_start + vec_num_offset));
        //kprintf(" Vec %h, Ori %h\n", set_vec_num_after, set_vec_num_before);
        
        // Set IDT Entry
        idt_entry(cur_vec,  DA_IntGate, (void *)cur_hdlr_start, PRIVILEGE_KRNL);
    }
    
    // Initialize the paremter of IDTR
    int_idt.idtr_value.base = (u32)(&int_idt.entries);
    int_idt.idtr_value.limit = IDT_ENTRY_COUNT * sizeof(struct idt_gate) - 1;;
    
    // Load IDT
    init_idt_mp();
    
    kprintf(" Done!\n");
}
