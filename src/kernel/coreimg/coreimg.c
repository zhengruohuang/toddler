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


void init_coreimg()
{
    kprintf("Initializing core image\n");
    
    coreimg_addr = hal->coreimg_load_addr;
    header = (struct coreimg_header *)hal->coreimg_load_addr;
    files = (struct coreimg_record *)(hal->coreimg_load_addr + sizeof(struct coreimg_header));
    
    int has_next = 0;
    int cur_index = 0;
    char name_buf[32];
    
    do {
        has_next = get_next_core_file_name(cur_index, name_buf, sizeof(name_buf));
        kprintf("\tFile #%d: %s\n", cur_index, name_buf);
        cur_index++;
    } while (has_next);
}

int get_core_file_count()
{
    return header->file_count;
}

int get_next_core_file_name(int index, char *buf, size_t buf_size)
{
    int i;
    
    for (i = index; i < header->file_count; i++) {
        assert(buf_size > strlen((char *)files[i].file_name));
        strcpy(buf, (char *)files[i].file_name);
        
        return i != header->file_count - 1;
    }
    
    return 0;
}

int has_core_file(char *name)
{
    int i;
    
    for (i = 0; i < header->file_count; i++) {
        if (!strcmp((char *)files[i].file_name, name)) {
            return 1;
        }
    }
        
    return 0;
}

void *load_core_file(char *name)
{
    int i;
    
    for (i = 0; i < header->file_count; i++) {
        if (!strcmp((char *)files[i].file_name, name)) {
            return (void *)(coreimg_addr + files[i].start_offset);
        }
    }
    
    return NULL;
}
