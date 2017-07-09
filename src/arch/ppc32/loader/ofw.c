#include "common/include/data.h"
#include "loader/ofw.h"
#include "loader/mempool.h"


static ofw_phandle_t ofw_chosen;
static ofw_ihandle_t ofw_stdout;
static ofw_phandle_t ofw_root;
static ofw_ihandle_t ofw_mmu;
static ofw_ihandle_t ofw_memory_prop;
static ofw_phandle_t ofw_memory;

static ofw_entry_t ofw_cif;


/*
 * Helper functions
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

static int strcmp(const char *s1, const char *s2)
{
    int result = 0;
    
    while (*s1 || *s2) {
        if (!s1 && !s2) {
            return 0;
        }
        
        if (*s1 != *s2) {
            return *s1 > *s2 ? 1 : -1;
        }
        
        s1++;
        s2++;
    }
    
    return result;
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

void ofw_printf(char *fmt, ...)
{
    char *c = fmt;
    
    va_list args;
    va_start(args, fmt);

    while (*c) {
        switch (*c) {
        case '%': {
            char token = *++c;
            switch (token) {
            case 'x':
            case 'h':
            case 'p': {
                ulong val = va_arg(args, ulong);
                puthex(val);
                break;
            }
            case 'd': {
                ulong val = va_arg(args, ulong);
                puthex(val);
                break;
            }
            case 's' : {
                char * val = va_arg(args, char *);
                putstr(val);
                break;
            }
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(token);
                break;
            }
            
            break;
        }
        case '\n':
        case '\r':
            putchar('\n');
            break;
        case '\t':
            putstr("    ");
            break;
        default:
            putchar(*c);
            break;
        }
        
        c++;
    }

    va_end(args);
}


/*
 * OFW wrappers
 */
static ofw_arg_t ofw_get_prop(const ofw_phandle_t device, const char *name, void *buf, const int len)
{
    return ofw_call("getprop", 4, 1, NULL, device, name, buf, len);
}

static ofw_phandle_t ofw_find_dev(const char *name)
{
    return (ofw_phandle_t)ofw_call("finddevice", 1, 1, NULL, name);
}

static ofw_arg_t ofw_package_to_path(ofw_phandle_t device, char *buf, const int len)
{
    return ofw_call("package-to-path", 3, 1, NULL, device, buf, len);
}

static ofw_arg_t ofw_get_proplen(const ofw_phandle_t device, const char *name)
{
    return ofw_call("getproplen", 2, 1, NULL, device, name);
}

static ofw_arg_t ofw_next_prop(const ofw_phandle_t device, char *previous, char *buf)
{
    return ofw_call("nextprop", 3, 1, NULL, device, previous, buf);
}

static ofw_phandle_t ofw_get_child_node(const ofw_phandle_t node)
{
    return (ofw_phandle_t)ofw_call("child", 1, 1, NULL, node);
}

static ofw_phandle_t ofw_get_peer_node(const ofw_phandle_t node)
{
    return (ofw_phandle_t)ofw_call("peer", 1, 1, NULL, node);
}


/*
 * OFW init
 */
void ofw_init(ulong ofw_entry)
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
 * Memory map
 */
static ulong mem_zone_buf[4];

int ofw_mem_zone(int idx, ulong *start, ulong *size)
{
    int bytes = (int)ofw_get_prop(ofw_memory, "reg", mem_zone_buf, sizeof(mem_zone_buf));
    if (bytes <= 0) {
        putstr("Error: Unable to get physical memory information\n");
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


/*
 * Address translation
 */
void *ofw_translate(void *virt)
{
    ofw_arg_t results[4];
    if (ofw_call("call-method", 4, 5, results, "translate", ofw_mmu, virt, 0)) {
        putstr("Error: mmu method translate failed\n");
        panic();
    }

    // If the translation failed then this address is probably directly mapped
    if (!results[0]) {
        return virt;
    }

#if (ARCH_WIDTH == 32)
        return (void *)(ulong)results[2];
#elif (ARCH_WIDTH == 64)
        return (void *)(((ulong)results[2] << 32) | (ulong)results[3]);
#else
#error Unsupported architecture width
        return NULL;
#endif
}


/*
 * Screen
 */
int ofw_screen_is_graphic()
{
    // Open screen device
    ofw_phandle_t ofw_screen = ofw_find_dev("screen");
    if (ofw_screen == (ofw_phandle_t)-1) {
        putstr("Unable to open screen device");
        panic();
    }
    
    // Get device type
    char device_type[OFW_TREE_PROPERTY_MAX_VALUELEN];
    int ret = (int)ofw_get_prop(ofw_screen, "device_type", device_type, OFW_TREE_PROPERTY_MAX_VALUELEN);
    if (ret <= 0) {
        return 0;
    }
    
    // Check device type
    device_type[OFW_TREE_PROPERTY_MAX_VALUELEN - 1] = '\0';
    if (strcmp(device_type, "display") != 0) {
        return 0;
    }
    
    return 1;
}

void ofw_fb_info(void **addr, int *width, int *height, int *depth, int *bpl)
{
    // Open screen device
    ofw_phandle_t ofw_screen = ofw_find_dev("screen");
    if (ofw_screen == (ofw_phandle_t)-1) {
        putstr("Unable to open screen device");
        panic();
    }
    
    // Get display info
    ofw_prop_t fb_addr, fb_width, fb_height, fb_depth, fb_bpl;
    
    if ((int)ofw_get_prop(ofw_screen, "address", &fb_addr, sizeof(fb_addr)) <= 0) {
        fb_addr = 0;
    }
    if ((int)ofw_get_prop(ofw_screen, "width", &fb_width, sizeof(fb_width)) <= 0) {
        fb_width = 0;
    }
    if ((int)ofw_get_prop(ofw_screen, "height", &fb_height, sizeof(fb_height)) <= 0) {
        fb_height = 0;
    }
    if ((int)ofw_get_prop(ofw_screen, "depth", &fb_depth, sizeof(fb_depth)) <= 0) {
        fb_depth = 0;
    }
    if ((int)ofw_get_prop(ofw_screen, "linebytes", &fb_bpl, sizeof(fb_bpl)) <= 0) {
        fb_bpl = 0;
    }
    
    // Return
    if (addr) *addr = (void *)fb_addr;
    if (width) *width = (int)fb_width;
    if (height) *height = (int)fb_height;
    if (depth) *depth = (int)fb_depth;
    if (bpl) *bpl = (int)fb_bpl;
}

void ofw_escc_info(void **addr)
{
    // Open screen device
    ofw_phandle_t ofw_screen = ofw_find_dev("screen");
    if (ofw_screen == (ofw_phandle_t)-1) {
        putstr("Unable to open screen device");
        panic();
    }
    
    // Get serial info
    ofw_prop_t serial_offset[2];
    if ((int)ofw_get_prop(ofw_screen, "reg", serial_offset, sizeof(serial_offset)) <= 0) {
        serial_offset[0] = 0;
    }
    
    // Find PCI node
    ofw_phandle_t child = ofw_get_child_node(ofw_root);
    while (child && child != (ofw_phandle_t)-1) {
        // Get device type
        char devt[OFW_TREE_PROPERTY_MAX_VALUELEN];
        int ret = (int)ofw_get_prop(child, "device_type", devt, OFW_TREE_PROPERTY_MAX_VALUELEN);
        if (ret > 0 && devt[0] == 'p' && devt[1] == 'c' && devt[2] == 'i') {
            break;
        }
        
        // Move to next node
        child = ofw_get_peer_node(child);
    }
    
    // Find PCI base address
    ofw_prop_t pci_base[2];
    pci_base[0] = 0;
    if (child && child != (ofw_phandle_t)-1) {
        ofw_get_prop(child, "reg", pci_base, sizeof(pci_base));
    }
    
    void *serial_addr = (void *)pci_base[0] + serial_offset[0];
    serial_addr = ofw_translate(serial_addr);
    
    // Return
    if (addr) {
        *addr = serial_addr;
    }
}


/*
 * Quiesce
 */
void ofw_quiesce()
{
    ofw_call("quiesce", 0, 0, NULL);
}


/*
 * Firmware memory allocation
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

static void *ofw_claim_phys_any(const int len, const int alignment)
{
    void *addr = NULL;
    int ret = -1;
    
#if (ARCH_WIDTH == 32)
    ofw_arg_t retaddr[1];
    ret = ofw_call("call-method", 5, 2, retaddr, "claim", ofw_memory_prop, alignment, len, (ofw_arg_t)0);
    addr = (void *)retaddr[0];
    
#elif (ARCH_WIDTH == 64)
    ofw_arg_t retaddr[2];
    ret = ofw_call("call-method", 6, 3, retaddr, "claim", ofw_memory_prop, alignment, len, (ofw_arg_t)0, (ofw_arg_t)0);
    addr = (void *)((retaddr[0] << 32) | retaddr[1]);
    
#else
#error Unsupported architecture width
#endif
    
    if (ret || !addr) {
        putstr("Error: memory method claim failed\n");
        panic();
    }
    
    return addr;
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

void ofw_alloc(void **virt, void **phys, const int size, int align)
{
    if (!align) {
        align = PAGE_SIZE;
    }
    
    *virt = ofw_claim_virt_any(size, align);
    *phys = ofw_claim_phys_any(size, align);
    
    ofw_map(*phys, *virt, ALIGN_UP(size, PAGE_SIZE), (ofw_arg_t)-1);
}


/*
 * Copy device tree
 */
static char path[OFW_TREE_PATH_MAX_LEN + 1];
static char name[OFW_TREE_PROPERTY_MAX_NAMELEN];
static char name2[OFW_TREE_PROPERTY_MAX_NAMELEN];

static void *ofw_tree_rebase(void *addr)
{
    return mempool_to_phys(addr);
}

static struct ofw_tree_node *ofw_tree_alloc_node()
{
    return (struct ofw_tree_node *)mempool_alloc(sizeof(struct ofw_tree_node), 0);
}

static struct ofw_tree_prop *ofw_tree_alloc_prop(int count)
{
    return (struct ofw_tree_prop *)mempool_alloc(count * sizeof(struct ofw_tree_prop), 0);
}

static char *ofw_tree_alloc_space(int size)
{
    char *addr = mempool_alloc(size + 1, 0);
    if (addr) {
        addr[size] = '\0';
    }
    
    return addr;
}

/*
 * Transfer information from one OpenFirmware node into its memory
 * representation.
 *
 * Transfer entire information from the OpenFirmware device tree 'current' node
 * to its memory representation in 'current_node'. This function recursively
 * processes all node's children. Node's peers are processed iteratively in
 * order to prevent stack from overflowing.
 *
 * @param current_node Pointer to uninitialized ofw_tree_node structure that will become the memory represenation of 'current'.
 * @param parent_node  Parent ofw_tree_node structure or NULL in case of root node.
 * @param current      OpenFirmware phandle to the current device tree node.
 *
 */
static void ofw_tree_node_process(struct ofw_tree_node *current_node, struct ofw_tree_node *parent_node, ofw_phandle_t current, int level)
{
    while (current_node) {
        // Init current node
        current_node->parent = (struct ofw_tree_node *)ofw_tree_rebase(parent_node);
        current_node->peer = NULL;
        current_node->child = NULL;
        current_node->handle = current;
        current_node->num_props = 0;
        current_node->prop = NULL;
        
        // Get the disambigued name
        int len = (int)ofw_package_to_path(current, path, OFW_TREE_PATH_MAX_LEN);
        if (len == -1) {
            return;
        }
        path[len] = '\0';
        
        // Find last slash and do not include the slash
        int i;
        for (i = len; (i > 0) && (path[i - 1] != '/'); i--);
        len -= i;
        
        // Add space for trailing '\0'
        char *da_name = ofw_tree_alloc_space(len + 1);
        if (!da_name) {
            return;
        }
        
        memcpy(da_name, &path[i], len);
        da_name[len] = '\0';
        current_node->name = (char *)ofw_tree_rebase(da_name);
        
//         // Tell user about this node
//         int j;
//         for (j = 0; j < level; j++) {
//             putstr("  ");
//         }
//         ofw_printf("Node: %s\n", da_name[0] ? da_name : "(root)");
        
        // Recursively process the potential child node.
        ofw_phandle_t child = ofw_get_child_node(current);
        if ((child != 0) && (child != (ofw_phandle_t)-1)) {
            struct ofw_tree_node *child_node = ofw_tree_alloc_node();
            if (child_node) {
                ofw_tree_node_process(child_node, current_node, child, level + 1);
                current_node->child = (struct ofw_tree_node *)ofw_tree_rebase(child_node);
            }
        }
        
        // Count properties
        name[0] = '\0';
        while (ofw_next_prop(current, name, name2) == 1) {
            current_node->num_props++;
            memcpy(name, name2, OFW_TREE_PROPERTY_MAX_NAMELEN);
        }
        
        if (!current_node->num_props) {
            return;
        }
        
        // Copy properties
        struct ofw_tree_prop *property = ofw_tree_alloc_prop(current_node->num_props);
        if (!property) {
            return;
        }
        
        name[0] = '\0';
        for (i = 0; ofw_next_prop(current, name, name2) == 1; i++) {
            if (i == current_node->num_props) {
                break;
            }
            
            memcpy(name, name2, OFW_TREE_PROPERTY_MAX_NAMELEN);
            memcpy(property[i].name, name, OFW_TREE_PROPERTY_MAX_NAMELEN);
            property[i].name[OFW_TREE_PROPERTY_MAX_NAMELEN - 1] = '\0';
            
            int size = (int)ofw_get_proplen(current, name);
            property[i].size = size;
            
            if (size) {
                char *buf = ofw_tree_alloc_space(size);
                if (buf) {
                    // Copy property value to memory node.
                    ofw_get_prop(current, name, buf, size);
                    property[i].value = ofw_tree_rebase(buf);
                }
            } else {
                property[i].value = NULL;
            }
            
            //ofw_printf(" %s\n", property[i].name);
        }
        
        // Just in case we ran out of memory
        current_node->num_props = i;
        current_node->prop = (struct ofw_tree_prop *)ofw_tree_rebase(property);
        
        // Iteratively process the next peer node
        ofw_phandle_t peer = ofw_get_peer_node(current);
        if ((peer != 0) && (peer != (ofw_phandle_t)-1)) {
            struct ofw_tree_node *peer_node = ofw_tree_alloc_node();
            if (peer_node) {
                current_node->peer = (struct ofw_tree_node *)ofw_tree_rebase(peer_node);
                current_node = peer_node;
                current = peer;
                
                // Process the peer in next iteration
                continue;
            }
        }
        
        // No more peers on this level
        break;
    }
}

static void ofw_tree_node_extra(struct ofw_tree_node *root, char *name)
{
    ofw_phandle_t handle = ofw_find_dev(name);
    if (handle != (ofw_phandle_t)-1) {
        struct ofw_tree_node *node = ofw_tree_alloc_node();
        if (node) {
            ofw_tree_node_process(node, root, handle, 1);
            node->peer = root->child;
            root->child = (struct ofw_tree_node *)ofw_tree_rebase(node);
        }
    }
}

/*
 * Construct memory representation of OpenFirmware device tree
 */
struct ofw_tree_node *ofw_tree_build()
{
    putstr("Copying device tree\n");
    
    struct ofw_tree_node *root = ofw_tree_alloc_node();
    if (root) {
        ofw_tree_node_process(root, NULL, ofw_root, 0);
    }
    
    // The firmware client interface does not automatically include the "ssm" node in the list of children of "/"
    ofw_tree_node_extra(root, "/ssm@0,0");
    
    putstr("Done\n");
    
    return (struct ofw_tree_node *)ofw_tree_rebase(root);
}
