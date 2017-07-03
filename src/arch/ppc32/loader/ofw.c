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
static ofw_arg_t ofw_get_prop(const ofw_phandle_t device, const char *name, void *buf, const int len)
{
    return ofw_call("getprop", 4, 1, NULL, device, name, buf, len);
}

static ofw_phandle_t ofw_find_dev(const char *name)
{
    return (ofw_phandle_t)ofw_call("finddevice", 1, 1, NULL, name);
}

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
static ulong mem_zone_buf[32];

int ofw_get_mem_zone(int idx, ulong *start, ulong *size)
{
    int bytes = (int)ofw_get_prop(ofw_memory, "reg", mem_zone_buf, sizeof(mem_zone_buf));
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
        //putstr("Error: Unable to translate virtual address @ ");
        //puthex((ulong)virt);
        //panic();
    }

    if (sizeof(long) == sizeof(int)) {
        // 32-bit
        return (void *)(ulong)results[2];
    } else {
        // 64-bit
        //return (void *)(((ulong)results[2] << 32) | (ulong)results[3]);
        return NULL;
    }
}

void ofw_test_translation()
{
    putstr("Virtual @ ");
    puthex((ulong)ofw_translate);
    putstr(" -> physical @ ");
    puthex((ulong)ofw_translate(ofw_translate));
    putstr("\n");
    
    putstr("Virtual @ ");
    puthex((ulong)ofw_cif);
    putstr(" -> physical @ ");
    puthex((ulong)ofw_translate(ofw_cif));
    putstr("\n");
}


/*
 * Screen
 */
struct ofw_display ofw_displays[8];
int ofw_display_count = 0;

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

static void ofw_setup_display(ofw_phandle_t handle)
{
    // Check for device type
    char device_type[OFW_TREE_PROPERTY_MAX_VALUELEN];
    int ret = (int)ofw_get_prop(handle, "device_type", device_type, OFW_TREE_PROPERTY_MAX_VALUELEN);
    if (ret <= 0) {
        return;
    }
    
    device_type[OFW_TREE_PROPERTY_MAX_VALUELEN - 1] = '\0';
    if (strcmp(device_type, "display") != 0) {
        return;
    }
    
    // See if there is enough space
    if (ofw_display_count >= sizeof(ofw_displays) / sizeof(struct ofw_display)) {
        return;
    }
    
    // Get display info
    ofw_prop_t fb_addr, fb_width, fb_height, fb_depth, fb_bytes_per_line;
    
    if ((int)ofw_get_prop(handle, "address", &fb_addr, sizeof(fb_addr)) <= 0) {
        fb_addr = 0;
    }
    if ((int)ofw_get_prop(handle, "width", &fb_width, sizeof(fb_width)) <= 0) {
        fb_width = 0;
    }
    if ((int)ofw_get_prop(handle, "height", &fb_height, sizeof(fb_height)) <= 0) {
        fb_height = 0;
    }
    if ((int)ofw_get_prop(handle, "depth", &fb_depth, sizeof(fb_depth)) <= 0) {
        fb_depth = 0;
    }
    if ((int)ofw_get_prop(handle, "linebytes", &fb_bytes_per_line, sizeof(fb_bytes_per_line)) <= 0) {
        fb_bytes_per_line = 0;
    }
    
    putstr("Frame buffer @ ");
    puthex((ulong)fb_addr);
    putstr(", Width: ");
    puthex((ulong)fb_width);
    putstr(", Height: ");
    puthex((ulong)fb_height);
    putstr(", Depth: ");
    puthex((ulong)fb_depth);
    putstr(", Bytes per Line: ");
    puthex((ulong)fb_bytes_per_line);
    putstr("\n");
    
    // Save this display
    ofw_displays[ofw_display_count].addr = (unsigned long)fb_addr;
    ofw_displays[ofw_display_count].width = (int)fb_width;
    ofw_displays[ofw_display_count].height = (int)fb_height;
    ofw_displays[ofw_display_count].depth = (int)fb_depth;
    ofw_displays[ofw_display_count].bytes_per_line = (int)fb_bytes_per_line;
    
    ofw_display_count++;
}

static ofw_phandle_t ofw_get_child_node(const ofw_phandle_t node)
{
    return (ofw_phandle_t)ofw_call("child", 1, 1, NULL, node);
}

static ofw_phandle_t ofw_get_peer_node(const ofw_phandle_t node)
{
    return (ofw_phandle_t)ofw_call("peer", 1, 1, NULL, node);
}

static void ofw_detect_displays(ofw_phandle_t current)
{
    while ((current != 0) && (current != (ofw_phandle_t)-1)) {
        // Set up current device if it's a display
        ofw_setup_display(current);
        
        // Recursively process the potential child node
        ofw_phandle_t child = ofw_get_child_node(current);
        if ((child != 0) && (child != (ofw_phandle_t)-1)) {
            ofw_detect_displays(child);
        }
        
        /*
         * Iteratively process the next peer node.
         * Note that recursion is a bad idea here.
         * Due to the topology of the OpenFirmware device tree,
         * the nesting of peer nodes could be to wide and the
         * risk of overflowing the stack is too real.
         */
        ofw_phandle_t peer = ofw_get_peer_node(current);
        if ((peer != 0) && (peer != (ofw_phandle_t)-1)) {
            current = peer;
            // Process the peer in next iteration.
            continue;
        }
        
        // No more peers on this level.
        break;
    }
}

void ofw_setup_displays()
{
    putstr("To setup displays\n");
    ofw_detect_displays(ofw_root);
}


/*
 * Quiesce
 */
void ofw_quiesce()
{
    ofw_call("quiesce", 0, 0, NULL);
}


/*
 * Copy device tree
 */
#define OFW_TREE_ALLOC_POOL_SIZE    32768
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))

static char path[OFW_TREE_PATH_MAX_LEN + 1];
static char name[OFW_TREE_PROPERTY_MAX_NAMELEN];
static char name2[OFW_TREE_PROPERTY_MAX_NAMELEN];

static char ofw_tree_alloc_pool[OFW_TREE_ALLOC_POOL_SIZE];
static int ofw_tree_alloc_offset = 0;

static void *ofw_tree_alloc(int size, int align)
{
    if (align) {
        size = ALIGN_UP(size, align);
    }
    
    if (ofw_tree_alloc_offset + size > OFW_TREE_ALLOC_POOL_SIZE) {
        putstr("Tree allocator run of of memory!\n");
        return NULL;
    }
    
    void *addr = &ofw_tree_alloc_pool + ofw_tree_alloc_offset;
    ofw_tree_alloc_offset += size;
    
    return addr;
}

static void *ofw_tree_rebase(void *addr)
{
    return addr;
}

static struct ofw_tree_node *ofw_tree_alloc_node()
{
    return (struct ofw_tree_node *)ofw_tree_alloc(sizeof(struct ofw_tree_node), 4);
}

static struct ofw_tree_prop *ofw_tree_alloc_prop(int count)
{
    return (struct ofw_tree_prop *)ofw_tree_alloc(count * sizeof(struct ofw_tree_prop), 4);
}

static char *ofw_tree_alloc_space(int size)
{
    char *addr = ofw_tree_alloc(size + 1, 4);
    if (addr) {
        addr[size] = '\0';
    }
    
    return addr;
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

static void memzero(void *src, int size)
{
    int i;
    char *ptr = (char *)src;
    
    for (i = 0; i < size; i++) {
        *(ptr++) = 0;
    }
    
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
static void ofw_tree_node_process(struct ofw_tree_node *current_node, struct ofw_tree_node *parent_node, ofw_phandle_t current)
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
        
        // Recursively process the potential child node.
        ofw_phandle_t child = ofw_get_child_node(current);
        if ((child != 0) && (child != (ofw_phandle_t)-1)) {
            struct ofw_tree_node *child_node = ofw_tree_alloc_node();
            if (child_node) {
                ofw_tree_node_process(child_node, current_node, child);
                current_node->child = (struct ofw_tree_node *)ofw_tree_rebase(child_node);
            }
        }
        
        // Count properties
        name[0] = '\0';
        while (ofw_next_prop(current, name, name2) == 1) {
            current_node->num_props++;
            memcpy(name, name2, OFW_TREE_PROPERTY_MAX_NAMELEN);
        }
        
        // Tell user about this node
        putstr("Node: ");
        putstr(current_node->name);
        putstr(", num props: ");
        puthex(current_node->num_props);
        putstr("\n");
        
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
            
            // Tell user about this prop
            putstr("  Prop: ");
            if (is_ascii(property[i].name[0])) {
                putstr(property[i].name);
            } else {
                puthex(*(unsigned long *)property[i].name);
            }
            putstr(", value: ");
            if (is_ascii(((char *)property[i].value)[0]) && '`' != ((char *)property[i].value)[0]) {
                putstr((char *)property[i].value);
            } else {
                puthex(*(unsigned long *)property[i].value);
            }
            putstr("\n");
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

/*
 * Construct memory representation of OpenFirmware device tree
 */
struct ofw_tree_node *ofw_tree_build()
{
    memzero(&ofw_tree_alloc_pool, OFW_TREE_ALLOC_POOL_SIZE);
    
    struct ofw_tree_node *root = ofw_tree_alloc_node();
    if (root) {
        ofw_tree_node_process(root, NULL, ofw_root);
    }
    
    // The firmware client interface does not automatically include the "ssm" node in the list of children of "/"
    ofw_phandle_t ssm_node = ofw_find_dev("/ssm@0,0");
    if (ssm_node != (ofw_phandle_t)-1) {
        struct ofw_tree_node *ssm = ofw_tree_alloc_node();
        if (ssm) {
            ofw_tree_node_process(ssm, root, ssm_node);
            ssm->peer = root->child;
            root->child = (struct ofw_tree_node *)ofw_tree_rebase(ssm);
        }
    }
    
    return (struct ofw_tree_node *)ofw_tree_rebase(root);
}
