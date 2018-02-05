#ifndef __ARCH_PPC32_LOADER_INCLUDE_OFW__
#define __ARCH_PPC32_LOADER_INCLUDE_OFW__


/*
 * OFW services
 */
extern void ofw_init(ulong ofw_entry);
extern void ofw_printf(char *fmt, ...);

extern int ofw_mem_zone(int idx, ulong *start, ulong *size);

extern int ofw_screen_is_graphic();
extern void ofw_fb_info(void **addr, int *width, int *height, int *depth, int *bpl);
extern void ofw_escc_info(void **addr);

extern ulong ofw_find_int_ctrl_base();

extern void *ofw_translate(void *virt);

extern void ofw_quiesce();

extern void ofw_alloc(void **virt, void **phys, const int size, int align);

extern struct ofw_tree_node *ofw_tree_build();


#endif
