#ifndef __KERNEL_INCLUDE_COREIMG__
#define __KERNEL_INCLUDE_COREIMG__


#include "common/include/data.h"


/*
 * Core image
 */
extern void init_coreimg();
extern int get_core_file_count();
extern int get_core_file_name(int index, char *buf, size_t buf_size);
extern int get_core_file_index(const char *name);
extern ulong get_core_file_size(int index);
extern void *get_core_file_addr_by_index(int index);
extern void *get_core_file_addr_by_name(char *name);


/*
 * FS
 */
extern void init_coreimgfs();


/*
 * Start up
 */
extern void startup_process_started(ulong proc_id);
extern void start_user();


#endif
