#include "common/include/data.h"
#include "loader/include/lib.h"
#include "loader/include/print.h"


static int initialized = 0;

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
    
    initialized = 1;
}

void *mempool_alloc(int size, int align)
{
    if (!initialized) {
        lprintf("Mempool not initialized!\n");
        panic();
    }
    
    if (align < sizeof(ulong)) {
        align = sizeof(ulong);
    }
    
    if (align) {
        size = ALIGN_UP(size, align);
    }
    
    if (mempool_offset + size > mempool_size) {
        return NULL;
    }
    
    ulong result = mempool_virt_base + mempool_offset;
    mempool_offset += size;
    
    return (void *)result;
}

void *mempool_to_phys(void *virt)
{
    if (!initialized) {
        lprintf("Mempool not initialized!\n");
        panic();
    }
    
    ulong result = (ulong)virt;
    
    if (mempool_phys_base > mempool_virt_base) {
        result += mempool_phys_base - mempool_virt_base;
    } else {
        result -= mempool_virt_base - mempool_phys_base;
    }
    
    return (void *)result;
}
