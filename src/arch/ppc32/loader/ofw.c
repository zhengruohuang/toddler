#include "common/include/data.h"
#include "loader/ofw.h"


static ofw_phandle_t ofw_chosen;
static ofw_ihandle_t ofw_stdout;
static ofw_phandle_t ofw_root;
static ofw_ihandle_t ofw_mmu;
static ofw_ihandle_t ofw_memory_prop;
static ofw_phandle_t ofw_memory;

static ofw_entry_t ofw_cif;


/*
 * Other helper functions
 */
static void panic()
{
    while (1);
}

/*
 * Perform a call to OpenFirmware client interface
 *
 *  service String identifying the service requested.
 *  nargs   Number of input arguments.
 *  nret    Number of output arguments. This includes the return value.
 *  rets    Buffer for output arguments or NULL. The buffer must accommodate nret - 1 items.
 *
 *  return  Return value returned by the client interface.
 *
 */
static ofw_arg_t ofw_call(const char *service, const int nargs, const int nret, ofw_arg_t *rets, ...)
{
    struct ofw_args args;
    args.service = (ofw_arg_t)service;
    args.nargs = nargs;
    args.nret = nret;
    
    va_list list;
    va_start(list, rets);
    
    int i;
    for (i = 0; i < nargs; i++)
        args.args[i] = va_arg(list, ofw_arg_t);
    
    va_end(list);
    
    for (i = 0; i < nret; i++)
        args.args[i + nargs] = 0;
    
    ofw_cif(&args);
    
    for (i = 1; i < nret; i++)
        rets[i - 1] = args.args[i + nargs];
    
    return args.args[nargs];
}


/*
 * Print
 */
static void ofw_putchar(const char ch)
{
    if (ofw_stdout == 0) {
        return;
    }
    
    ofw_call("write", 3, 1, NULL, ofw_stdout, &ch, 1);
}


static int is_ascii(char ch)
{
    return ch >= 32 && ch <= 126;
}

static void putchar(const char ch)
{
    if (ch == '\n' || ch == '\r') {
        ofw_putchar('\n');
    } else if (is_ascii(ch)) {
        ofw_putchar(ch);
    } else {
        ofw_putchar('?');
    }
}

static void putstr(char *s)
{
    char ch = *s;
    while (ch) {
        putchar(ch);
        ch = *++s;
    };
}

static void puthex(ulong hex)
{
    char buf[9];
    int i;

    for (i = 0; i < 8; i++) {
        int digit = (int)(hex & 0xf);
        buf[7 - i] = (digit > 9) ? ('a' + (digit - 10)) : ('0' + digit);
        hex >>= 4;
    };
    buf[8] = '\0';

    putstr(buf);
}


/*
 * OFW init
 */
static ofw_arg_t get_prop(const ofw_phandle_t device, const char *name, void *buf, const int len)
{
    return ofw_call("getprop", 4, 1, NULL, device, name, buf, len);
}

static ofw_phandle_t find_dev(const char *name)
{
    return (ofw_phandle_t)ofw_call("finddevice", 1, 1, NULL, name);
}

void ofw_init(ulong ofw_entry)
{
    int ret = 0;
    
    ofw_cif = (ofw_entry_t)ofw_entry;
    
    ofw_chosen = find_dev("/chosen");
    if (ofw_chosen == (ofw_phandle_t)-1) {
        panic();
    }
    
    ret = (int)get_prop(ofw_chosen, "stdout", &ofw_stdout, sizeof(ofw_stdout));
    if (ret <= 0) {
        ofw_stdout = 0;
    }
    
    ofw_root = find_dev("/");
    if (ofw_root == (ofw_phandle_t)-1) {
        putstr("Unable to find device @ /\n");
        panic();
    }
    
    ret = (int)get_prop(ofw_chosen, "mmu", &ofw_mmu, sizeof(ofw_mmu));
    if (ret <= 0) {
        putstr("Unable to get mmu property\n");
        panic();
    }
    
    ret = (int)get_prop(ofw_chosen, "memory", &ofw_memory_prop, sizeof(ofw_memory_prop));
    if (ret <= 0) {
        putstr("Unable to get memory property\n");
        panic();
    }
    
    ofw_memory = find_dev("/memory");
    if (ofw_memory == (ofw_phandle_t)-1) {
        putstr("Unable to find device @ /memory\n");
        panic();
    }
    
    putstr("Open firmware initialized\n");
}


/*
 * Memory map
 */
static ulong mem_zone_buf[32];

int ofw_get_mem_zone(int idx, ulong *start, ulong *size)
{
    int bytes = (int)get_prop(ofw_memory, "reg", mem_zone_buf, sizeof(mem_zone_buf));
    if (bytes <= 0) {
        putstr("Error: Unable to get physical memory information, halting.\n");
        panic();
    }
    
    int cell_size = 2;
    int cell_idx = idx * cell_size;
    if (cell_idx * sizeof(ulong) >= bytes) {
        return -1;
    }
    
    *start = mem_zone_buf[cell_idx];
    *size = mem_zone_buf[cell_idx + 1];
    
    return 0;
}

void ofw_print_mem_zones()
{
    ulong start = 0;
    ulong size = 0;
    int idx = 0;
    
    int ret = ofw_get_mem_zone(idx++, &start, &size);
    do {
        putstr("Memory zone start @ ");
        puthex(start);
        putstr(", size: ");
        puthex(size);
        putstr("\n");
        
        ret = ofw_get_mem_zone(idx++, &start, &size);
    } while (!ret);
}
