#ifndef __ARCH_PPC32_LOADER_INCLUDE_OFW__
#define __ARCH_PPC32_LOADER_INCLUDE_OFW__


/*
 * Arg list
 */
typedef __builtin_va_list va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)


/*
 * OFW data types
 */
#define MEMMAP_MAX_RECORDS  32
#define MAX_OFW_ARGS        12

typedef unsigned long ofw_arg_t;
typedef unsigned long ofw_prop_t;
typedef unsigned long ofw_ihandle_t;
typedef unsigned long ofw_phandle_t;

struct ofw_args {
    ofw_arg_t service;      // Command name
    ofw_arg_t nargs;        // Number of in arguments
    ofw_arg_t nret;         // Number of out arguments
    ofw_arg_t args[MAX_OFW_ARGS];   // List of arguments
};

typedef ofw_arg_t (*ofw_entry_t)(struct ofw_args *args);


/*
 * OFW services
 */
extern void ofw_init(ulong ofw_entry);
extern int ofw_get_mem_zone(int idx, ulong *start, ulong *size);
extern void ofw_print_mem_zones();
extern void *ofw_translate(void *virt);
extern void ofw_test_translation();

#endif
