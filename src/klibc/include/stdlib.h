#ifndef __KLIBC_INCLUDE_STDLIB__
#define __KLIBC_INCLUDE_STDLIB__


#include "common/include/data.h"


/*
 * Heap alloc
 */
#define HALLOC_CHUNK_SIZE   (32 * 1024)

extern void init_halloc();
extern void *halloc(size_t size);
extern void hfree(void *addr);


/*
 * Struct alloc
 */


/*
 * Malloc
 */
extern void init_malloc();
extern void *malloc(size_t size);
extern void *calloc(int count, size_t size);
extern void free(void *size);


#endif
