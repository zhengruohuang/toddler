#ifndef __ARCH_IA32_COMMON_INCLUDE_FLOPPYIMG__
#define __ARCH_IA32_COMMON_INCLUDE_FLOPPYIMG__


#include "common/include/data.h"


struct floppy_fat_header {
    u16 file_count;
    u16 fat_count;
    
    struct {
        u16 major;
        u16 minor;
        u16 revision;
        u16 release;
    } version;
    
    u32 create_time;
} packedstruct;

struct floppy_fat_entry {
    u16 start_sector;
    u16 sector_count;
    u8  file_name[12];
} packedstruct;

struct floppy_fat_master {
    struct floppy_fat_header header;
    struct floppy_fat_entry entries[31];
} packedstruct;

struct floppy_fat_slave {
    struct floppy_fat_entry entries[32];
} packedstruct;


#endif
