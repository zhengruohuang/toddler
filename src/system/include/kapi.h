#ifndef __SYSTEM_INCLUDE_KAPI__
#define __SYSTEM_INCLUDE_KAPI__

#include "common/include/data.h"
#include "common/include/syscall.h"


/*
 * Init KAPI
 */
extern void init_kapi();


/*
 * URS
 */
extern asmlinkage void urs_reg_super_handler(msg_t *s);
extern asmlinkage void urs_reg_op_handler(msg_t *s);

extern asmlinkage void urs_open_handler(msg_t *s);
extern asmlinkage void urs_close_handler(msg_t *s);
extern asmlinkage void urs_read_handler(msg_t *s);
extern asmlinkage void urs_write_handler(msg_t *s);
extern asmlinkage void urs_list_handler(msg_t *s);

extern asmlinkage void urs_create_handler(msg_t *s);
extern asmlinkage void urs_remove_handler(msg_t *s);
extern asmlinkage void urs_rename_handler(msg_t *s);

extern asmlinkage void urs_stat_handler(msg_t *s);


#endif
