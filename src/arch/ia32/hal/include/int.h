#ifndef __ARCH_IA32_INCLUDE_INT__
#define __ARCH_IA32_INCLUDE_INT__


#include "common/include/data.h"


// Number of IDT entries
#define IDT_ENTRY_COUNT             256

// Privilige Levels
#define PRIVILEGE_KRNL              0
#define PRIVILEGE_USER              3

// GDT selector
#define SELECTOR_RUN                8

// Pre-defined hardware IRQs
#define IRQ_CLOCK                   0
#define IRQ_KEYBOARD                1
#define IRQ_CASCADE                 2       // cascade enable for 2nd AT controller
#define IRQ_ETHER                   3       // default ethernet interrupt vector
#define IRQ_SECONDARY               3       // RS232 interrupt vector for port 2
#define IRQ_RS232                   4       // RS232 interrupt vector for port 1
#define IRQ_XT_WINI                 5       // xt winchester
#define IRQ_FLOPPY                  6       // floppy disk
#define IRQ_PRINTER                 7
#define IRQ_AT_WINI                 14      // at winchester

// Pre-defined trap/fault interrupt vectors
#define VEC_DIVIDE                  0x0
#define VEC_DEBUG                   0x1
#define VEC_NMI                     0x2
#define VEC_BREAKPOINT              0x3
#define VEC_OVERFLOW                0x4
#define VEC_BOUNDS                  0x5
#define VEC_INVAL_OP                0x6
#define VEC_COPROC_NOT              0x7
#define VEC_DOUBLE_FAULT            0x8
#define VEC_COPROC_SEG              0x9
#define VEC_INVAL_TSS               0xA
#define VEC_SEG_NOT                 0xB
#define VEC_STACK_FAULT             0xC
#define VEC_PROTECTION              0xD
#define VEC_PAGE_FAULT              0xE
#define VEC_COPROC_ERR              0x10
#define VEC_ALIGN_CHECK             0x11
#define VEC_MACHINE_CHECK           0x12
#define VEC_SIMD                    0x13

// Pre-defined device interrupt vectors
#define VEC_IRQ_START               0x20

// System call interupts
#define INT_VECTOR_SYS_CALL         0x7F

// User's interrupt vectors
#define VEC_USER_START              0x80

// Gate types
#define DA_IntGate                  0x8E


struct idt_gate {
    u16     offset_low;                     // Offset Low
    u16     selector;                       // Selector
    u8      dcount;                         // This field should always be zero in IDT
    u8      attr;                           // P(1) DPL(2) DT(1) TYPE(4)
    u16     offset_high;                    // Offset High
} packedstruct;                             // A IDT gate

struct idt_reg {
    u16     limit;
    u32     base;
} packedstruct;

struct idt {
    struct idt_reg  idtr_value;
    struct idt_reg  entries[IDT_ENTRY_COUNT];
} packedstruct;


typedef int (*int_handler)(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags);


/*
 * Assembly export functions and addresses
 */
extern void int_handler_divide();
extern void int_handler_debug();
extern void int_handler_nmi();
extern void int_handler_breakpoint();
extern void int_handler_overflow();
extern void int_handler_bounds_check();
extern void int_handler_invalid_opcode();
extern void int_handler_copr_not_available();
extern void int_handler_double_fault();
extern void int_handler_copr_seg_overrun();
extern void int_handler_invalid_tss();
extern void int_handler_segment_not_present();
extern void int_handler_stack_exception();
extern void int_handler_general_protection();
extern void int_handler_page_fault();
extern void int_handler_copr_error();
extern void int_handler_copr_error();
extern void int_handler_align_check();
extern void int_handler_machine_check();
extern void int_handler_simd_error();

extern void int_handler_general();
extern void int_handler_template_begin();
extern void int_handler_template_end();
extern void int_handlers();


/*
 * Turn on/off LOCAL interrupts
 */
extern void disable_local_int();
extern void enable_local_int();


/*
 * Pre-defined interrupt handlers
 */
extern int_handler int_handler_list[IDT_ENTRY_COUNT];
extern void init_int_handlers();
extern int asmlinkage int_handler_entry(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags);
extern int int_handler_dummy(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags);
extern int int_handler_exception(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags);
extern int int_handler_device(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags);


/*
 * IDT functions
 */
extern void init_idt();
extern void init_idt_mp();


/*
 * Interrupt vectors
 */
#define INT_VECTOR_ALLOC_START        32
#define INT_VECTOR_ALLOC_END          255

enum int_vector_state {
    int_vector_unknown,
    int_vector_reserved,
    int_vector_free,
    int_vector_allocated,
    int_vector_other
};

extern void init_int_vector();
extern int alloc_int_vector(int_handler hdlr);
extern void free_int_vector(int vector);


#endif
