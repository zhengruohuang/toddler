#include "loader/periph.h"


/*
 * Helper functions
 */
static void panic()
{
    lprintf("Panic!");
    
    while (1) {
        // do nothing
    }
}


/*
 * BSS
 */
extern int __bss_start;
extern int __bss_end;

static void init_bss()
{
    int *cur;
    
    for (cur = &__bss_start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}


/*
 * Main
 */
void main()
{
    init_bss();
    init_periph();
    
    lprintf("Welcome to Toddler!\n");
    
    while (1) {
        lprintf("Should never arrive here!\n");
        panic();
    }
}
