#include "common/include/data.h"
#include "common/include/proc.h"
#include "common/include/errno.h"
#include "common/include/urs.h"
#include "common/include/ua.h"
#include "klibc/include/stdio.h"
#include "klibc/include/stdlib.h"
#include "klibc/include/string.h"
#include "klibc/include/stdstruct.h"
#include "klibc/include/assert.h"
#include "klibc/include/time.h"
#include "klibc/include/sys.h"
#include "system/include/urs.h"
#include "system/include/fs.h"


struct procfs_node {
};

void init_procfs()
{
    init_process_monitor();
}
