#ifndef __ARCH_MIPS32_HAL_INCLUDE_VECNUM__
#define __ARCH_MIPS32_HAL_INCLUDE_VECNUM__


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
 * Vector numbers for external interrupts
 */
#define INT_VECTOR_EXTERNAL_BASE    36


#endif
