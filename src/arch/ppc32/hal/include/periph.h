#ifndef __ARCH_PPC32_HAL_INCLUDE_PERIPH__
#define __ARCH_PPC32_HAL_INCLUDE_PERIPH__


#include "common/include/ofw.h"


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
