#include "common/include/data.h"
#include "driver/include/devfs.h"


static struct dev_tree_node *sysbus_node = NULL;


void init_sysbus()
{
    sysbus_node = dev_tree_create(DEV_NODE_BUS, "sysbus");
}
