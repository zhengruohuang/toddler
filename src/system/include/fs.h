#ifndef __SYSTEM_INCLUDE_FS__
#define __SYSTEM_INCLUDE_FS__


#include "common/include/data.h"


/*
 * RAM FS
 */
extern void init_ramfs();
extern int register_ramfs(char *path);
extern int unregister_ramfs(char *path);
extern void test_ramfs();


/*
 * FAT 12/16/32
 */


/*
 * EXT 2/3
 */


#endif
