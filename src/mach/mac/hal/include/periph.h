#ifndef __ARCH_PPC32_HAL_INCLUDE_PERIPH__
#define __ARCH_PPC32_HAL_INCLUDE_PERIPH__


#include "common/include/ofw.h"
#include "hal/include/vector.h"


/*
 * OFW
 */
extern struct ofw_tree_node *ofw_node_get_child(struct ofw_tree_node *node);
extern struct ofw_tree_node *ofw_node_get_peer(struct ofw_tree_node *node);
extern struct ofw_tree_node *ofw_node_find_by_name(struct ofw_tree_node *node, char *name);
extern struct ofw_tree_prop *ofw_prop_find(struct ofw_tree_node *node, char *name);
extern void init_ofw();


/*
 * Frame buffer
 */
extern void fb_draw_char_ppc(char ch);
extern void init_fb();


/*
 * ESCC serial controller
 */
extern void escc_draw_char(char ch);
extern void start_escc();
extern void init_escc();


/*
 * Heathrow PIC
 */
extern void heathrow_pic_eoi(int wired);
extern int heathrow_pic_register_wired(int wired, int_handler handler);
extern int heathrow_pic_get_vector();
extern void start_heathrow_pic();
extern void init_heathrow_pic();


/*
 * Print
 */
extern void draw_char(char ch);
extern void init_print();


/*
 * Top level
 */
extern void init_periph();
extern void start_periph();

extern int pic_get_vector();
extern void pic_eoi(int wired);
extern int pic_register_wired(int wired, int_handler handler);


#endif
