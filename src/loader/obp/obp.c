#include "common/include/data.h"
#include "loader/include/lib.h"
#include "loader/include/print.h"
#include "loader/include/obp.h"


static volatile struct openboot_prom *obp;

int obp_root_handle;
char obp_buf[MAX_OBP_BUF_LEN];


/*
 * Device operations
 */
int obp_open(char *name)
{
    return obp->dev_ops_v2.dev_open(name);
}

void obp_close(int dev)
{
    obp->dev_ops_v2.dev_close(dev);
}

int obp_inst2pkg(int dev)
{
    return obp->dev_ops_v2.inst2pkg(dev);
}


/*
 * Node operations
 */
int obp_get_prop(int dev, char *name, void *buf, int len)
{
    int ret = obp->node_ops->get_prop(dev, name, obp_buf);
    if (ret > 0 && buf) {
        memcpy(buf, obp_buf, len);
    }
    
    return ret;
}

int obp_find_node(char *name)
{
    int sub_handle = 0;
    
    // See the child nodes in the parent
    sub_handle = obp->node_ops->child(obp_root_handle);
    while (sub_handle) {
        // Get this handle's name
        int name_prop = obp_get_prop(sub_handle, "name", obp_buf, MAX_OBP_BUF_LEN);
        if (name_prop) {
            // Compare it's name
            if (!strcmp(obp_buf, name)) {
                return sub_handle;
            }
        }
        
        // Move to next
        sub_handle = obp->node_ops->next_node(sub_handle);
    }
    
    return 0;
}

int obp_find_child_node(int parent, char *name)
{
    int sub_handle = 0;
    
    // See the child nodes in the parent
    sub_handle = obp->node_ops->child(parent);
    while (sub_handle) {
        // Get this handle's name
        int name_prop = obp_get_prop(sub_handle, "name", obp_buf, MAX_OBP_BUF_LEN);
        if (name_prop) {
            // Compare it's name
            if (!strcmp(obp_buf, name)) {
                return sub_handle;
            }
        }
        
        // Move to next
        sub_handle = obp->node_ops->next_node(sub_handle);
    }
    
    // Move to subsequent child nodes
    sub_handle = obp->node_ops->child(parent);
    while (sub_handle) {
        int match_handle = obp_find_child_node(sub_handle, name);
        if (match_handle) {
            return match_handle;
        }
        
        // Move to next
        sub_handle = obp->node_ops->next_node(sub_handle);
    }
    
    return 0;
}


/*
 * Print
 */
void obp_putstr(char *s)
{
    obp->putstr(s, strlen(s));
}

void obp_draw_char(char c)
{
    char s[2] = { c, '\0' };
    obp->putstr(s, 2);
}


/*
 * Power
 */
static void reboot(char *msg)
{
    obp->reboot(msg);
}


/*
 * Memory
 */
static ulong mem_zone_buf[12];

int obp_physmem_zone(int idx, ulong *start, ulong *size)
{
    int handle = obp_find_node("memory");
    if (!handle) {
        obp_putstr("Unable to find node @ memory\n");
        panic();
    }
    
    int bytes = (int)obp_get_prop(handle, "available", mem_zone_buf, sizeof(mem_zone_buf));
    if (bytes <= 0) {
        obp_putstr("Unable to get avail physical memory map\n");
        panic();
    }
    
    int cell_size = 3;
    int cell_idx = idx * cell_size;
    if (cell_idx * sizeof(ulong) >= bytes) {
        return -1;
    }
    
    *start = mem_zone_buf[cell_idx + 1];
    if (size) *size = mem_zone_buf[cell_idx + 2];
    
    return 0;
}

int obp_virtmem_zone(int idx, ulong *start, ulong *size)
{
    int handle = obp_find_node("virtual-memory");
    if (!handle) {
        obp_putstr("Unable to find node @ virtual-memory\n");
        panic();
    }
    
    int bytes = (int)obp_get_prop(handle, "available", mem_zone_buf, sizeof(mem_zone_buf));
    if (bytes <= 0) {
        obp_putstr("Unable to get avail virtual memory map\n");
        panic();
    }
    
    int cell_size = 3;
    int cell_idx = idx * cell_size;
    if (cell_idx * sizeof(ulong) >= bytes) {
        return -1;
    }
    
    if (start) *start = mem_zone_buf[cell_idx + 1];
    if (size) *size = mem_zone_buf[cell_idx + 2];
    
    return 0;
}



/*
 * Memory alloc
 */
void *obp_find_good_pzone()
{
    ulong guess = (ulong)&obp_find_good_pzone;
    ulong start = 0;
    int idx = 0;
    int err = obp_physmem_zone(idx, &start, NULL);
    while (!err) {
        if (start > guess) {
            return (void *)start;
        }
        
        idx++;
        err = obp_physmem_zone(idx, &start, NULL);
    }
    
    return NULL;
}

void *obp_alloc(const int size, int align)
{
    // Find the zone to use
    void *suggest = obp_find_good_pzone();
    
    void *vaddr = obp->dev_ops_v2.dumb_mem_alloc(suggest, size);
    return vaddr;
}


/*
 * Device tree
 */
struct obp_tree_node {
};

struct obp_tree_node *obp_duplicate_tree()
{
}


/*
 * Info
 */
void show_obp_info()
{
    lprintf("OpenBoot PROM @ %p\n", obp);
    lprintf("OpenBoot PROM version: %d, plugin rev: %d, print rev: %d\n",
        obp->rom_ver, obp->plugin_rev, obp->print_rev
    );
}


/*
 * OFW init
 */
void init_obp(ulong obp_entry)
{
    obp = (void *)obp_entry;
    
    int root_dev = obp_open("/");
    obp_root_handle = obp_inst2pkg(root_dev);
    obp_close(root_dev);
    
    obp_putstr("OpenBoot PROM initialized!\n");
}
