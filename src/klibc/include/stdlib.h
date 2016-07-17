#ifndef __KLIBC_INCLUDE_STDLIB__
#define __KLIBC_INCLUDE_STDLIB__


#include "common/include/data.h"


/*
 * Heap alloc
 */
#define HALLOC_CHUNK_SIZE   (32 * 1024)

extern void init_halloc();
extern void *halloc();
extern void hfree(void *addr);


/*
 * Struct alloc
 */
typedef void (*salloc_callback_t)(void* entry);

extern void init_salloc();
extern int salloc_create(size_t size, size_t align, salloc_callback_t construct, salloc_callback_t destruct);
extern void *salloc(int obj_id);
extern void sfree(void *ptr);


/*
 * Malloc
 */
extern void init_malloc();
extern void *malloc(size_t size);
extern void *calloc(int count, size_t size);
extern void free(void *size);
extern void test_malloc();


#endif
