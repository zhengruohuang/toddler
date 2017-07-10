#include "common/include/data.h"
#include "common/include/memory.h"
#include "common/include/bootparam.h"
#include "hal/include/lib.h"
#include "hal/include/print.h"


static struct pht_group *pht;
static struct pht_attri_group *attri;


void init_pht()
{
    struct boot_parameters *bp = get_bootparam();
    
    kprintf("Initializing PHT\n");
    
    pht = (void *)bp->pht_addr;
    attri = (void *)bp->attri_addr;
    
    kprintf("\tPHT @ %p, attributes @ %p\n", pht, attri);
}
