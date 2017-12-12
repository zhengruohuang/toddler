#include "common/include/data.h"
#include "common/include/urs.h"
#include "klibc/include/stdstruct.h"
#include "driver/include/devfs.h"


/*
 * Initialize the driver
 */
static int init()
{
}

/*
 * Register/unregister a device
 */
static int dev_plugg(unsigned long node_id, struct dev_desc *desc)
{
}

static int dev_eject(unsigned long node_id, struct dev_desc *desc)
{
}

static int dev_gone(unsigned long node_id, struct dev_desc *desc)
{
}


/*
 * Regular device operations
 */
static int open(struct dev_desc *desc, struct open_desc *inst)
{
}

static int close(struct dev_desc *desc, struct open_desc *inst)
{
}

static int read(struct dev_desc *desc, struct open_desc *inst, void *buf, unsigned long count, unsigned long *actual)
{
}

static int write(struct dev_desc *desc, struct open_desc *inst, void *buf, unsigned long count, unsigned long *actual)
{
}

static int seek(struct dev_desc *desc, struct open_desc *inst, u64 offset, enum urs_seek_from from, u64 *newpos)
{
}

static int ioctl(struct dev_desc *desc, struct open_desc *inst, unsigned long action, unsigned long value)
{
}


/*
 * Register the driver
 */
int register_pseudo_null()
{
}
