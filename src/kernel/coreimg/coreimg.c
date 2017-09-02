/*
 * Core image
 */


#include "common/include/data.h"
#include "common/include/coreimg.h"
#include "kernel/include/hal.h"
#include "kernel/include/lib.h"
#include "kernel/include/coreimg.h"


static ulong coreimg_addr;
static struct coreimg_header *header;
static struct coreimg_record *files;


static u32 swap_endian32(u32 val)
{
    u32 rr = val & 0xff;
    u32 rl = (val >> 8) & 0xff;
    u32 lr = (val >> 16) & 0xff;
    u32 ll = (val >> 24) & 0xff;
    
    u32 swap = (rr << 24) | (rl << 16) | (lr << 8) | ll;
    return swap;
}

void init_coreimg()
{
    kprintf("Initializing core image @ %p\n", (void *)hal->coreimg_load_addr);
    
    coreimg_addr = hal->coreimg_load_addr;
    header = (struct coreimg_header *)hal->coreimg_load_addr;
    files = (struct coreimg_record *)(hal->coreimg_load_addr + sizeof(struct coreimg_header));
    
    int i;
    int has_next = 0;
    int cur_index = 0;
    char name_buf[32];
    
    // Swap endian if necessary
    if (ARCH_BIG_ENDIAN != header->big_endian) {
        header->file_count = swap_endian32(header->file_count);
        header->image_size = swap_endian32(header->image_size);
        
        for (i = 0; i < header->file_count; i++) {
            files[i].start_offset = swap_endian32(files[i].start_offset);
            files[i].length = swap_endian32(files[i].length);
        }
    }
    
    // Get all files
    do {
        has_next = get_core_file_name(cur_index, name_buf, sizeof(name_buf));
        kprintf("\tFile #%d: %s\n", cur_index, name_buf);
        cur_index++;
    } while (has_next);
}

int get_core_file_count()
{
    return header->file_count;
}

int get_core_file_name(int index, char *buf, size_t buf_size)
{
    int i;
    
    for (i = index; i < header->file_count; i++) {
        assert(buf_size > strlen((char *)files[i].file_name));
        strcpy(buf, (char *)files[i].file_name);
        
        return i != header->file_count - 1;
    }
    
    return 0;
}

ulong get_core_file_size(int index)
{
    if (index >= header->file_count) {
        return 0;
    }
    
    return files[index].length;
}

int get_core_file_index(const char *name)
{
    int i;
    
    for (i = 0; i < header->file_count; i++) {
        if (!strcmp((char *)files[i].file_name, name)) {
            return i;
        }
    }
        
    return -1;
}

void *get_core_file_addr_by_index(int index)
{
    if (index >= header->file_count) {
        return NULL;
    }
    
    return (void *)(coreimg_addr + files[index].start_offset);
}

void *get_core_file_addr_by_name(char *name)
{
    int i;
    
    for (i = 0; i < header->file_count; i++) {
        if (!strcmp((char *)files[i].file_name, name)) {
            return (void *)(coreimg_addr + files[i].start_offset);
        }
    }
    
    return NULL;
}
