#ifndef __LOADER_PRINT_H__
#define __LOADER_PRINT_H__


#include "common/include/data.h"


extern void lprintf(char *fmt, ...);

extern void init_framebuffer_draw(void *f, int w, int h, int d, int p);
extern void framebuffer_draw_char(char ch);


#endif
