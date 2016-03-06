#ifndef __ARCH_IA32_I8259A_INCLUDE_I8259A__
#define __ARCH_IA32_I8259A_INCLUDE_I8259A__


#include "common/include/data.h"


// Hardware interrupts
#define I8259A_NR_IRQ                   16      // Number of IRQs
#define I8259A_CLOCK_IRQ                0
#define I8259A_KEYBOARD_IRQ             1
#define I8259A_CASCADE_IRQ              2       // cascade enable for 2nd AT controller
#define I8259A_ETHER_IRQ                3       // default ethernet interrupt vector
#define I8259A_SECONDARY_IRQ            3       // RS232 interrupt vector for port 2
#define I8259A_RS232_IRQ                4       // RS232 interrupt vector for port 1
#define I8259A_XT_WINI_IRQ              5       // xt winchester
#define I8259A_FLOPPY_IRQ               6       // floppy disk
#define I8259A_PRINTER_IRQ              7
#define I8259A_AT_WINI_IRQ              14      // at winchester

// I8259A interrupt controller ports.
#define I8259A_M_CTL                    0x20    // I/O port for interrupt controller         <Master>
#define I8259A_M_CTLMASK                0x21    // setting bits in this port disables ints   <Master>
#define I8259A_S_CTL                    0xA0    // I/O port for second interrupt controller  <Slave> 
#define I8259A_S_CTLMASK                0xA1    // setting bits in this port disables ints   <Slave> 

// Exception Vectors
#define I8259A_VECTOR_DIVIDE            0x0
#define I8259A_VECTOR_DEBUG             0x1
#define I8259A_VECTOR_NMI               0x2
#define I8259A_VECTOR_BREAKPOINT        0x3
#define I8259A_VECTOR_OVERFLOW          0x4
#define I8259A_VECTOR_BOUNDS            0x5
#define I8259A_VECTOR_INVAL_OP          0x6
#define I8259A_VECTOR_COPROC_NOT        0x7
#define I8259A_VECTOR_DOUBLE_FAULT      0x8
#define I8259A_VECTOR_COPROC_SEG        0x9
#define I8259A_VECTOR_INVAL_TSS         0xA
#define I8259A_VECTOR_SEG_NOT           0xB
#define I8259A_VECTOR_STACK_FAULT       0xC
#define I8259A_VECTOR_PROTECTION        0xD
#define I8259A_VECTOR_PAGE_FAULT        0xE
#define I8259A_VECTOR_COPROC_ERR        0x10

// IRQ Vectors
#define I8259A_VECTOR_IRQ0              0x20
#define I8259A_VECTOR_IRQ8              0x28

// System Call Vector
#define I8259A_VECTOR_SYS_CALL          0x90


extern void init_i8259a();
extern void start_i8259a();
extern void disable_i8259a();
extern void i8259a_enable_irq(int irq_num);
extern void i8259a_disable_irq(int irq_num);
extern void i8259a_enable_irq_all();
extern void i8259a_disable_irq_all();


#endif
