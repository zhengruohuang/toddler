#include "common/include/data.h"
#include "common/include/bootparam.h"
#include "common/include/coreimg.h"


/*
 * Herlper functions
 */
static void panic()
{
    while (1);
}

static void memcpy(void *dest, const void *src, int count)
{
    int i;
    
    char *s = (char *)src;
    char *d = (char *)dest;
    
    for (i = 0; i < count; i++) {
        *(d++) = *(s++);
    }
}

static void memzero(void *src, int size)
{
    int i;
    char *ptr = (char *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = 0;
    }
    
}

static int is_ascii(char ch)
{
    return ch >= 32 && ch <= 126;
}


/*
 * BSS
 */
extern int __bss_start;
extern int __bss_end;

static void init_bss()
{
    int *cur;
    
    for (cur = &__bss_start; cur < &__bss_end; cur++) {
        *cur = 0;
    }
}


/*
 * OFW
 */
typedef __builtin_va_list   va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)

typedef ulong ofw_arg_t;
typedef ulong ofw_prop_t;
typedef ulong ofw_ihandle_t;
typedef ulong ofw_phandle_t;

#define MAX_OFW_ARGS        12

struct ofw_args {
    ofw_arg_t service;      // Command name
    ofw_arg_t nargs;        // Number of in arguments
    ofw_arg_t nret;         // Number of out arguments
    ofw_arg_t args[MAX_OFW_ARGS];   // List of arguments
};

typedef ofw_arg_t (*ofw_entry_t)(struct ofw_args *args);


static ofw_phandle_t ofw_chosen = -1;
static ofw_ihandle_t ofw_stdout = -1;
static ofw_phandle_t ofw_root = -1;
static ofw_ihandle_t ofw_mmu = -1;
static ofw_ihandle_t ofw_memory_prop = -1;
static ofw_phandle_t ofw_memory = -1;

static ofw_entry_t ofw_cif = (void *)-1;


/*
 * Perform a OFW call
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
    for (i = 0; i < nargs; i++) {
        args.args[i] = va_arg(list, ofw_arg_t);
    }
    
    va_end(list);
    
    for (i = 0; i < nret; i++) {
        args.args[i + nargs] = 0;
    }
    
    ofw_cif(&args);
    
    for (i = 1; i < nret; i++) {
        rets[i - 1] = args.args[i + nargs];
    }
    
    return args.args[nargs];
}


/*
 * OFW print
 */
static void ofw_putchar(const char ch)
{
    if (ofw_stdout == 0) {
        return;
    }
    
    ofw_call("write", 3, 1, NULL, ofw_stdout, &ch, 1);
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
static ofw_arg_t ofw_get_prop(const ofw_phandle_t device, const char *name, void *buf, const int len)
{
    return ofw_call("getprop", 4, 1, NULL, device, name, buf, len);
}

static ofw_phandle_t ofw_find_dev(const char *name)
{
    return (ofw_phandle_t)ofw_call("finddevice", 1, 1, NULL, name);
}

static void ofw_init(ulong ofw_entry)
{
    int ret = 0;
    
    ofw_cif = (ofw_entry_t)ofw_entry;
    
    ofw_chosen = ofw_find_dev("/chosen");
    if (ofw_chosen == (ofw_phandle_t)-1) {
        panic();
    }
    
    ret = (int)ofw_get_prop(ofw_chosen, "stdout", &ofw_stdout, sizeof(ofw_stdout));
    if (ret <= 0) {
        ofw_stdout = 0;
    }
    
    ofw_root = ofw_find_dev("/");
    if (ofw_root == (ofw_phandle_t)-1) {
        putstr("Unable to find device @ /\n");
        panic();
    }
    
    ret = (int)ofw_get_prop(ofw_chosen, "mmu", &ofw_mmu, sizeof(ofw_mmu));
    if (ret <= 0) {
        putstr("Unable to get mmu property\n");
        panic();
    }
    
    ret = (int)ofw_get_prop(ofw_chosen, "memory", &ofw_memory_prop, sizeof(ofw_memory_prop));
    if (ret <= 0) {
        putstr("Unable to get memory property\n");
        panic();
    }
    
    ofw_memory = ofw_find_dev("/memory");
    if (ofw_memory == (ofw_phandle_t)-1) {
        putstr("Unable to find device @ /memory\n");
        panic();
    }
    
    putstr("Open firmware initialized\n");
}


/*
 * OFW memory allocation
 */
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))
#define PAGE_SIZE 4096

static void *ofw_claim_virt_any(const int len, const int alignment)
{
    ofw_arg_t addr;
    int ret = (int)ofw_call("call-method", 5, 2, &addr, "claim", ofw_mmu, alignment, len, (ofw_arg_t)0);
    if (ret || !(int)addr) {
        putstr("Error: mmu method claim failed\n");
        panic();
    }
    
    return (void *)addr;
}

static void *ofw_claim_phys(const void *phys, const int len, const int alignment)
{
    void *addr = NULL;
    int ret = -1;
    
#if (ARCH_WIDTH == 32)
    ofw_arg_t retaddr[1];
    ret = ofw_call("call-method", 5, 2, retaddr, "claim", ofw_memory_prop,
                   alignment, len, (ofw_arg_t)(ulong)phys);
    addr = (void *)(ulong)retaddr[0];
    
#elif (ARCH_WIDTH == 64)
    ofw_arg_t retaddr[2];
    ret = ofw_call("call-method", 6, 3, retaddr, "claim", ofw_memory_prop,
                   alignment, len,
                   (ofw_arg_t)((ulong)phys >> 32),
                   (ofw_arg_t)((ulong)phys & 0xffffffff));
    addr = (void *)(((ulong)retaddr[0] << 32) | (ulong)retaddr[1]);
    
#else
#error Unsupported architecture width
#endif
    
    if (ret || !addr) {
        putstr("Error: memory method claim failed\n");
        panic();
    }
    
    return addr;
}

static void *ofw_claim_phys_any(const int len, const int alignment)
{
    return ofw_claim_phys(0, len, alignment);
}

static void ofw_map(const void *phys, const void *virt, const int size, const ofw_arg_t mode)
{
    ofw_arg_t phys_hi;
    ofw_arg_t phys_lo;
    
#if (ARCH_WIDTH == 32)
    phys_hi = (ofw_arg_t)phys;
    phys_lo = 0;
    
#elif (ARCH_WIDTH == 64)
    phys_hi = (ofw_arg_t)phys >> 32;
    phys_lo = (ofw_arg_t)phys & 0xffffffff;
    
#else
#error Unsupported architecture width
#endif
    
    ofw_arg_t ret = ofw_call("call-method", 7, 1, NULL, "map", ofw_mmu, mode, ALIGN_UP(size, PAGE_SIZE), virt, phys_hi, phys_lo);
    
    if (ret != 0) {
        putstr("Error: Unable to map ");
        puthex((ulong)virt);
        putstr(" to ");
        puthex((ulong)phys);
        putstr(" (size: ");
        puthex((ulong)size);
        putstr(")\n");
        
        panic();
    }
}

static void ofw_alloc_any(void **base, void **base_pa, const int size)
{
    *base = ofw_claim_virt_any(size, PAGE_SIZE);
    *base_pa = ofw_claim_phys_any(size, PAGE_SIZE);
    ofw_map(*base_pa, *base, ALIGN_UP(size, PAGE_SIZE), (ofw_arg_t)-1);
}

static void ofw_alloc(void **base, const void *base_pa, const int size)
{
    void *allocated_pa = ofw_claim_phys(base_pa, size, PAGE_SIZE);
    if (allocated_pa != base_pa) {
        putstr("Unable to claim phys @ ");
        puthex((ulong)base_pa);
        putstr("\n");
        
        putstr("Returned @ ");
        puthex((ulong)allocated_pa);
        
        //panic();
    }
    
    *base = ofw_claim_virt_any(size, PAGE_SIZE);
    ofw_map(base_pa, *base, ALIGN_UP(size, PAGE_SIZE), (ofw_arg_t)-1);
}


/*
 * Loader
 */
#define LOADER_ENTRY    0x100000

typedef void (*loader_entry_t)(ulong ofw_entry);
static loader_entry_t loader_entry = (void *)-1;

extern long __end;

static void load_images()
{
    // Get loader size
    char *loader_start = (char *)(&__bss_end) + 4;
    int loader_size = *(&__end);
    
    putstr("Loader @ ");
    puthex((ulong)loader_start);
    putstr(", size: ");
    puthex(loader_size);
    putstr("\n");
    
    // Get core image size
    char *coreimg_start = loader_start + loader_size;
    struct coreimg_header *hdr = (struct coreimg_header *)coreimg_start;
    int coreimg_size = hdr->image_size;
    
    putstr("Core image @ ");
    puthex((ulong)coreimg_start);
    putstr(", size: ");
    puthex(coreimg_size);
    putstr("\n");
    
    // Allocate buffer
    char *buf = NULL;
    ofw_alloc((void **)&buf, (void *)LOADER_ENTRY, loader_size + coreimg_size);
    
    // Copy loader data
    memcpy(buf, loader_start, loader_size);
    
    // Copy core image data
    memcpy(buf + loader_size, coreimg_start, coreimg_size);
}

static void jump_to_loader(ulong ofw_entry)
{
    loader_entry = (loader_entry_t)((ulong)&__end + 4);
    loader_entry(ofw_entry);
}


/*
 * Bootstrap entry
 */
void bootstrap(ulong ofw_entry)
{
    init_bss();
    ofw_init(ofw_entry);
    
    load_images();
    //jump_to_loader(ofw_entry);
    putstr("Here");
    
    while (1) {
        //panic();
    }
}
