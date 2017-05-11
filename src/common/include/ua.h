#ifndef __COMMON_INCLUDE_UA__
#define __COMMON_INCLUDE_UA__


#include "common/include/data.h"


/*
 * User ID
 */
#define UA_USER_ID_ROOT         0
#define UA_USER_ID_SYSTEM       1
#define UA_USER_ID_ALLOC_BASE   1000

/*
 * Group ID
 */
#define UA_GROUP_ID_ROOT        0
#define UA_GROUP_ID_SYSTEM      1
#define UA_GROUP_ID_ALLOC_BASE  1000

/*
 * Permissions
 */
#define UA_OWNER_PERM_NONE      0
#define UA_OWNER_PERM_EXEC      0x100
#define UA_OWNER_PERM_WRITE     0x200
#define UA_OWNER_PERM_READ      0x400
#define UA_OWNER_PERM_ALL       (UA_OWNER_PERM_EXEC | UA_OWNER_PERM_WRITE | UA_OWNER_PERM_READ)

#define UA_GROUP_PERM_NONE      0
#define UA_GROUP_PERM_EXEC      0x10
#define UA_GROUP_PERM_WRITE     0x20
#define UA_GROUP_PERM_READ      0x40
#define UA_GROUP_PERM_ALL       (UA_GROUP_PERM_EXEC | UA_GROUP_PERM_WRITE | UA_GROUP_PERM_READ)

#define UA_OTHER_PERM_NONE      0
#define UA_OTHER_PERM_EXEC      0x1
#define UA_OTHER_PERM_WRITE     0x2
#define UA_OTHER_PERM_READ      0x4
#define UA_OTHER_PERM_ALL       (UA_OTHER_PERM_EXEC | UA_OTHER_PERM_WRITE | UA_OTHER_PERM_READ)


#endif
