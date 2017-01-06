#ifndef __ARCH_MIPS32_HAL_INCLUDE_INT__
#define __ARCH_MIPS32_HAL_INCLUDE_INT__


#include "common/include/data.h"
#include "common/include/task.h"


/*
 * Vector numbers for internal exceptions
 */
#define INT_VECTOR_DUMMY            0

#define INT_VECTOR_TLB_READ_ONLY    1

#define INT_VECTOR_TLB_MISS_READ    2
#define INT_VECTOR_TLB_MISS_WRITE   3

#define INT_VECTOR_ADDR_ERR_READ    4
#define INT_VECTOR_ADDR_ERR_WRITE   5

#define INT_VECTOR_BUS_ERR_READ     6
#define INT_VECTOR_BUS_ERR_WRITE    7

#define INT_VECTOR_SYSCALL          8
#define INT_VECTOR_BREAK            9
#define INT_VECTOR_TRAP             13

#define INT_VECTOR_CP_UNUSABLE      11

#define INT_VECTOR_INT_OVERFLOW     12
#define INT_VECTOR_FP_EXCEPT        15
#define INT_VECTOR_CP2_EXCEPT       16

#define INT_VECTOR_WATCH            23
#define INT_VECTOR_MACHINE_CHECK    24
#define INT_VECTOR_THREAD           25

#define INT_VECTOR_CACHE_ERR        30

#define INT_VECTOR_INSTR_MDMX       22
#define INT_VECTOR_INSTR_DSP        26

/*
 * Vector numbers for internal interrupts
 */
#define INT_VECTOR_LOCAL_TIMER      32
#define INT_VECTOR_PAGE_FAULT       33


/*
 * Interrupt handler
 */
struct int_context {
    ulong vector;
    ulong error_code;
    
    struct context *context;
};

typedef int (*int_handler)(struct int_context *intc, struct kernel_dispatch_info *kdi);


/*
 * Interrupt vectors
 */
#define INT_VECTOR_ALLOC_START      64
#define INT_VECTOR_ALLOC_END        255
#define INT_VECTOR_COUNT            (INT_VECTOR_ALLOC_END + 1)

enum int_vector_state {
    int_vector_unknown,
    int_vector_reserved,
    int_vector_free,
    int_vector_allocated,
    int_vector_other
};

extern int_handler int_handler_list[INT_VECTOR_COUNT];

extern void init_int_vector();
extern int set_int_vector(int vector, int_handler hdlr);
extern int alloc_int_vector(int_handler hdlr);
extern void free_int_vector(int vector);


/*
 * Generic
 */
extern void init_int();

extern int get_local_int_state();
extern void set_local_int_state(int enabled);

extern int disable_local_int();
extern void enable_local_int();
extern void restore_local_int(int enabled);

extern void init_int_state_mp();
extern void init_int_state();

extern void tlb_refill_handler(struct context *context);
extern void cache_error_handler(struct context *context);
extern void general_except_handler(struct context *context);


/*
 * Entry
 */
extern void int_entry_wrapper_begin();
extern void int_entry_wrapper_end();


/*
 * Syscall
 */
extern void init_syscall();
extern void set_syscall_return(struct context *context, int succeed, ulong return0, ulong return1);


#endif
