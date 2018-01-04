#ifndef __HAL_INCLUDE_FB__
#define __HAL_INCLUDE_FB__


#include "common/include/data.h"


extern void fb_draw_char(char ch);
extern void init_fb_draw_char(void *f, int w, int h, int d, int p);


#endif

