#include "common/include/data.h"


#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))


static ulong mempool_virt_base;
static ulong mempool_phys_base;
static int mempool_size;
static int mempool_offset;


void mempool_init(void *virt, void *phys, int size)
{
    mempool_virt_base = (ulong)virt;
    mempool_phys_base = (ulong)phys;
    mempool_size = size;
    mempool_offset = 0;
}

void *mempool_alloc(int size, int align)
{
    if (align < sizeof(ulong)) {
        align = sizeof(ulong);
    }
    
    if (align) {
        size = ALIGN_UP(size, align);
    }
    
    if (mempool_offset + size > mempool_size) {
        return NULL;
    }
    
    ulong result = mempool_virt_base;
    mempool_virt_base += size;
    
    return (void *)result;
}

void *mempool_to_phys(void *virt)
{
    ulong result = (ulong)virt;
    
    if (mempool_phys_base > mempool_virt_base) {
        result += mempool_phys_base - mempool_virt_base;
    } else {
        result -= mempool_virt_base - mempool_phys_base;
    }
    
    return (void *)result;
}
