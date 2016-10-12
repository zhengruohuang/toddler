#ifndef __COMMON_INCLUDE_COREIMG__
#define __COMMON_INCLUDE_COREIMG__


#include "common/include/data.h"


struct coreimg_header {
    u32     file_count;
    u32     image_size;
    struct {
        u16 major;
        u16 minor;
        u16 revision;
        u16 release;
    }       version;
    u8      architecture;
    u8      build_type;
    u8      reserved[6];
    u64     time_stamp;
} packedstruct;

struct coreimg_record {
    u8      file_type;
    u8      load_type;
    u8      compressed;
    u8      reserved;
    u32     start_offset;
    u32     length;
    u8      file_name[20];
} packedstruct;

struct coreimg_fat {
    struct coreimg_header   header;
    struct coreimg_record   records[];
};


#endif
