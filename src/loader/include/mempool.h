#ifndef __LOADER_INCLUDE_MEMPOOL__
#define __LOADER_INCLUDE_MEMPOOL__


extern void mempool_init(void *virt, void *phys, int size);
extern void *mempool_alloc(int size, int align);
extern void *mempool_to_phys(void *virt);


#endif
