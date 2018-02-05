#ifndef __ARCH_ARMV7_HAL_INCLUDE_VECNUM__
#define __ARCH_ARMV7_HAL_INCLUDE_VECNUM__


/*
 * ARM exception vectors
 */
#define INT_VECTOR_DUMMY            0

#define INT_VECTOR_RESET            1
#define INT_VECTOR_UNDEFINED        2
#define INT_VECTOR_SVC              3
#define INT_VECTOR_FETCH            4
#define INT_VECTOR_DATA             5
#define INT_VECTOR_RESERVED         6
#define INT_VECTOR_IRQ              7
#define INT_VECTOR_FIQ              8


/*
 * Internal handlers
 */
#define INT_VECTOR_LOCAL_TIMER      10
#define INT_VECTOR_SYSCALL          11
#define INT_VECTOR_UNKNOWN_DEV      12
#define INT_VECTOR_SPURIOUS_FIQ     13

#endif
