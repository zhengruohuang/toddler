#ifndef __ARCH_SPARCV8_LOADER_INCLUDE_OBP__
#define __ARCH_SPARCV8_LOADER_INCLUDE_OBP__


/*
 * Reference
 *  https://github.com/qemu/openbios/blob/c5542f226c0d3d61e7bb578b70e591097d575479/arch/sparc32/openprom.h
 */


#include "common/include/data.h"


#define MAX_OBP_BUF_LEN    128

extern int obp_root_handle;
extern char obp_buf[MAX_OBP_BUF_LEN];


/*
 * Memory descriptors
 */
struct mlist_v0 {
    struct mlist_v0 *next;
    char *start_addr;
    unsigned bytes;
} packedstruct;

struct mem_desc {
    struct mlist_v0 * const *total_phys;
    struct mlist_v0 * const *prom_map;
    struct mlist_v0 * const *available; // What we can use
} packedstruct;

/*
 * Routines for traversing the prom device tree
 */
struct node_ops {
    int (*next_node)(int node);
    int (*child)(int node);
    int (*prop_len)(int node, const char *name);
    int (*get_prop)(int node, const char *name, char *val);
    int (*set_prop)(int node, const char *name, char *val, int len);
    const char * (*next_prop)(int node, const char *name);
} packedstruct;

struct dev_ops_v0 {
    int (*dev_open)(char *device_str);
    int (*dev_close)(int dev_desc);
    int (*dev_read_block)(int dev_desc, int num_blks, int blk_st, char *buf);
    int (*dev_write_block)(int dev_desc, int num_blks, int blk_st, char *buf);
    int (*dev_write_net)(int dev_desc, int num_bytes, char *buf);
    int (*dev_read_net)(int dev_desc, int num_bytes, char *buf);
    int (*dev_read_char)(int dev_desc, int num_bytes, int dummy, char *buf);
    int (*dev_write_char)(int dev_desc, int num_bytes, int dummy, char *buf);
    int (*dev_seek)(int dev_desc, long logical_offst, int from);
};

/*
 * Arguments sent to the kernel from the boot prompt
 */
struct boot_args_v0 {
    const char *argv[8];
    char args[100];
    char boot_dev[2];
    int boot_dev_ctrl;
    int boot_dev_unit;
    int dev_partition;
    const char *kernel_file_name;
    void *aieee1;
} packedstruct;

/*
 * V2 and up boot things
 */
struct boot_args_v2 {
    const char **path;
    const char **args;
    const int *stdin_fd;
    const int *stdout_fd;
} packedstruct;

/*
 * V2 and later PROM device operations
 */
struct dev_ops_v2 {
    // Convert ihandle to phandle
    int (*inst2pkg)(int d);
    char * (*dumb_mem_alloc)(char *va, unsigned sz);
    void (*dumb_mem_free)(char *va, unsigned sz);

    // To map devices into virtual I/O space
    char * (*dumb_mmap)(char *virta, int which_io, unsigned paddr, unsigned sz);
    void (*dumb_munmap)(char *virta, unsigned size);

    int (*dev_open)(char *devpath);
    void (*dev_close)(int d);
    int (*dev_read)(int d, char *buf, int nbytes);
    int (*dev_write)(int d, char *buf, int nbytes);
    int (*dev_seek)(int d, int hi, int lo);

    // Never issued (multistage load support)
    void (*wheee2)(void);
    void (*wheee3)(void);
};

/*
 * The top level PROM vector
 */
struct openboot_prom {
    // Version numbers
    unsigned int magic_cookie;
    unsigned int rom_ver;
    unsigned int plugin_rev;
    unsigned int print_rev;

    // Version 0 memory descriptors
    struct mem_desc mem_v0;

    // Node operations
    const struct node_ops *node_ops;

    char **boot_str;
    struct dev_ops_v0 dev_ops_v0;

    const char *stdin;
    const char *stdout;

    // Blocking getchar/putchar. NOT REENTRANT! (grr)
    int (*getchar)(void);
    void (*putchar)(int ch);

    // Non-blocking variants
    int (*getchar_nb)(void);
    int (*putchar_nb)(int ch);

    void (*putstr)(char *str, int len);

    // Misc
    void (*reboot)(char *bootstr);
    void (*printf)(__const__ char *fmt, ...);
    void (*abort)(void);
    __volatile__ unsigned int *ticks;
    void (*halt)(void);
    void (**sync_hook)(void);

    // Evaluate a forth string, not different proto for V0 and V2->up
    union {
        void (*eval_v0)(int len, char *str);
        void (*eval_v2)(char *str, int arg0, int arg1, int arg2, int arg3, int arg4);
    } forth_eval;

    const struct boot_args_v0 * const *boot_args_v0;

    // Get ether address
    unsigned int (*enaddr)(int d, char *enaddr);

    struct boot_args_v2 boot_args_v2;
    struct dev_ops_v2 dev_ops_v2;

    // PROM version 3 memory allocation
    char * (*mem_alloc_v3)(char *va, unsigned int size, unsigned int align);

    int reserved[14];

    // This one is sun4c/sun4 only
    void (*setctxt)(int ctxt, char *va, int pmeg);

    /*
     * PROM version 3 Multiprocessor routines. This stuff is crazy.
     * No joke. Calling these when there is only one cpu probably
     * crashes the machine, have to test this. :-)
     */

    // v3_cpustart() will start the cpu 'whichcpu' in mmu-context
    // 'thiscontext' executing at address 'prog_counter'
    int (*cpustart_v3)(unsigned int whichcpu, int ctxtbl_ptr, int thiscontext, char *prog_counter);

    // v3_cpustop() will cause cpu 'whichcpu' to stop executing
    // until a resume cpu call is made.
    int (*cpustop_v3)(unsigned int whichcpu);

    // v3_cpuidle() will idle cpu 'whichcpu' until a stop or
    // resume cpu call is made.
    int (*cpuidle_v3)(unsigned int whichcpu);

    // v3_cpuresume() will resume processor 'whichcpu' executing
    // starting with whatever 'pc' and 'npc' were left at the
    // last 'idle' or 'stop' call.
    int (*cpuresume_v3)(unsigned int whichcpu);
} packedstruct;


extern int obp_open(char *name);
extern void obp_close(int dev);
extern int obp_inst2pkg(int dev);

extern int obp_get_prop(int dev, char *name, void *buf, int len);
extern int obp_find_node(char *name);
extern int obp_find_child_node(int parent, char *name);

extern void obp_putstr(char *s);
extern void obp_draw_char(char c);

extern void *obp_alloc(const int size, int align);

extern int obp_physmem_zone(int idx, ulong *start, ulong *size);
extern int obp_virtmem_zone(int idx, ulong *start, ulong *size);

extern void show_obp_info();
extern void init_obp();


#endif
