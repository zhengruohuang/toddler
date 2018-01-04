#ifndef __LOADER_EXEC_H__
#define __LOADER_EXEC_H__


#include "common/include/data.h"
#include "common/include/coreimg.h"
#include "common/include/bootparam.h"


extern void load_images(struct coreimg_fat *coreimg, struct boot_parameters *bp,
                 ulong offset_up, ulong offset_down);


#endif
