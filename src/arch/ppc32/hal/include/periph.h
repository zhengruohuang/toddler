#ifndef __ARCH_PPC32_HAL_INCLUDE_PERIPH__
#define __ARCH_PPC32_HAL_INCLUDE_PERIPH__


/*
 * Frame buffer
 */
extern void fb_draw_char(char ch);
extern void init_fb();


/*
 * ESCC serial controller
 */
extern void escc_draw_char(char ch);
extern void init_escc();


/*
 * Print
 */
extern void draw_char(char ch);
extern void init_print();


#endif
