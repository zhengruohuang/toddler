#ifndef __KERNEL_INCLUDE_COREIMG__
#define __KERNEL_INCLUDE_COREIMG__


#include "common/include/data.h"


extern void init_coreimg();
extern int get_core_file_count();
extern int get_next_core_file_name(int index, char *buf, size_t buf_size);
extern int has_core_file(char *name);
extern void *load_core_file(char *name);


extern void start_user();


#endif
