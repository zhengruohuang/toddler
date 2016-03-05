#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/mps.h"


int mps_supported = 0;
ulong mps_lapic_addr = 0;

int mps_lapic_count = 0;
int mps_ioapic_count = 0;
int mps_bus_count = 0;
int mps_ioint_count = 0;
int mps_lint_count = 0;

static struct mps_fps *mps_fps;
static struct mps_ct *mps_ct;


static void lapic(struct mps_processor *pr, u32 i)
{
    kprintf("\t\tProcessor #%d: ID %d\n", i, pr->lapic_id);
}

static void bus(struct mps_bus *bus, u32 i)
{
    char buf[7];
    u32 buf_i;
    for (buf_i = 0; buf_i < 6; buf_i++) {
        buf[buf_i] = bus->bus_type[buf_i];
    }
    buf[6] = 0;
    
    kprintf("\t\tBus #%d: ID %d, %s\n", i, bus->bus_id, buf);
}

static void ioapic(struct mps_ioapic *ioa, u32 i)
{
    kprintf("\t\tIO APIC #%d: ID %d at %h\n", i, ioa->ioapic_id, ioa->ioapic_address);
}

static void ioint(struct mps_ioint *iointr, u32 i)
{
    char *int_type = NULL;
    
    switch (iointr->interrupt_type) {
    case 0:
        int_type = "INT";
        break;
    case 1:
        int_type = "NMI";
        break;
    case 2:
        int_type = "SMI";
        break;
    case 3:
        int_type = "Ext";
        break;
    default:
        int_type = "";
        break;
    }
    
    kprintf("\t\tIO-%s #%d: Pol %d, Tri %d, Bus %d, IRQ %d, IOA %d, Pin %d\n",
            int_type,
            i,
            iointr->polarity,
            iointr->trigger,
            iointr->src_bus_id,
            iointr->src_bus_irq,
            iointr->dest_ioapic_id,
            iointr->dest_ioapic_pin
    );
}

static void lint(struct mps_lintr *lintr, u32 i)
{
    char *int_type = NULL;
    
    switch (lintr->interrupt_type) {
        case 0:
            int_type = "INT";
            break;
        case 1:
            int_type = "NMI";
            break;
        case 2:
            int_type = "SMI";
            break;
        case 3:
            int_type = "Ext";
            break;
        default:
            int_type = "";
            break;
    }
    
    kprintf("\t\tL-%s #%d: Pol %d, Tri %d, Bus %d, IRQ %d, ID %d, Pin %d\n",
            int_type,
            i,
            lintr->polarity,
            lintr->trigger,
            lintr->src_bus_id,
            lintr->src_bus_irq,
            lintr->dest_lapic_id,
            lintr->dest_lapic_pin
    );
}

static int mps_configure()
{
    mps_lapic_addr = (u32)mps_ct->lapic_address;
    
    u8 *cur = &mps_ct->base_table[0];
    u16 i;
    
    // First we need to get the count of every entry
    for (i = 0; i < mps_ct->entry_count; i++) {
        switch (*cur) {
        case 0:
            // Processor entry
            lapic((struct mps_processor *)cur, mps_lapic_count++);
            cur += 20;
            break;
            
        case 1:
            // Bus entry
            bus((struct mps_bus*)cur, mps_bus_count++);
            cur += 8;
            break;
        
        case 2:
            // I/O APIC
            ioapic((struct mps_ioapic*)cur, mps_ioapic_count++);
            cur += 8;
            break;
        
        case 3:
            // I/O Interrupt Assignment
            ioint((struct mps_ioint*)cur, mps_ioint_count++);
            cur += 8;
            break;
        
        case 4:
            // Local Interrupt Assignment
            lint((struct mps_lintr*)cur, mps_lint_count++);
            cur += 8;
            break;
        
        default:
            // Something is wrong
            kprintf("\tMPS configuration table is bad: %d\n", *cur);
            return 0;
        }
    }
    
    // Process extended entries
    u8 *ext = (u8*)mps_ct + mps_ct->base_table_length;
    
    for (cur = ext; cur < ext + mps_ct->ext_table_length; cur += cur[MPS_CT_EXT_ENTRY_LEN]) {
        switch (cur[MPS_CT_EXT_ENTRY_TYPE]) {
        default:
            kprintf("\tSkipping MP Configuration Table extended entry type %d\n",
                    cur[MPS_CT_EXT_ENTRY_TYPE]);
        }
    }
    
    return 1;
}


static int default_table(int type)
{
    kprintf("\tMP Default Table has not been supported yet! Will configure as a single processor system\n");
    return 1;
}

static int check_config_table()
{
    u8 *base = (u8 *)mps_ct;
    u8 *ext = base + mps_ct->base_table_length;
    u8 sum;
    u16 i;
    
    // Compute the checksum for the base table
    for (i = 0, sum = 0; i < mps_ct->base_table_length; i++) {
        sum = (u8) (sum + base[i]);
    }
    
    if (sum) {
        return 0;
    }
    
    // Compute the checksum for the extended table
    for (i = 0, sum = 0; i < mps_ct->ext_table_length; i++) {
        sum = (u8) (sum + ext[i]);
    }
    
    return (sum == mps_ct->ext_table_checksum ? 1 : 0);
}

static int find_config_table()
{
    // MPS implemention on this system provides all necessary tables
    if ((mps_fps->config_type == 0) && (mps_fps->configuration_table)) {
        if (mps_fps->mpfib2 >> 7) {
            kprintf("\tMP PIC mode not supported\n");
        }
        
        mps_ct = (struct mps_ct *)(mps_fps->configuration_table);
        kernel_direct_map((ulong)mps_ct, 0);
        kernel_direct_map_array(
            (ulong)mps_ct,
            sizeof(struct mps_ct) + mps_ct->base_table_length +
            mps_ct->ext_table_length + mps_ct->oem_table_size,
            0
        );
        
        if (mps_ct->signature != MPS_CT_SIGNATURE) {
            kprintf("\tWrong MP Configuration Table Signature\n");
            return 0;
        }
        
        if (!check_config_table()) {
            kprintf("\tWrong MP Configuration Table Checksum\n");
            return 0;
        }
        
        kprintf("\tFound MP Configuration Table at %p\n", mps_ct);
        return 1;
    }
    
    // We need to construct a default table
    else {
        int construct_result = default_table(mps_fps->config_type);
        
        return construct_result;
    }
}

static int check_fps(u32 base)
{
    u32 i;
    u8 sum = 0;
    u8 *addr = (u8 *)base;
    
    for (i = 0, sum = 0; i < sizeof(struct mps_fps); i++) {
        sum += addr[i];
    }
    
    return (sum == 0 ? 1 : 0);
}

static int scan_mem_find_fps(ulong start, ulong len)
{
    int found = 0;
    ulong cur, end = start + len;
    
    for (cur = start; cur < end; cur += 16) {
        if (*((u32 *)cur) == MPS_FPS_SIGNATURE) {
            kernel_direct_map_array((ulong)mps_fps, sizeof(struct mps_fps), 0);
            
            if (check_fps(cur)) {
                mps_fps = (struct mps_fps *)cur;
                found = 1;
                break;
            }
        }
    }
    
    if (found) {
        kprintf("\tFound MP Floating Pointer Structure at %p\n", mps_fps);
    }
    
    return found;
}

static int find_fps()
{
    // Stage1: search first 1K of EBDA, if EBDA is undefined, search last 1K of base memory
    ulong cur_search_addr = (ulong)get_bootparam()->ebda_addr;
    int find_len = 1024;
    if (!cur_search_addr) {
        cur_search_addr = 639 * 1024;
    }
    
    int found = scan_mem_find_fps(cur_search_addr, find_len);
    
    if (!found) {
        // Stage2: 64K starting at 0xf0000
        cur_search_addr = 0xf0000;
        find_len = 64 * 1024;
        
        found = scan_mem_find_fps(cur_search_addr, find_len);
        
        if (!found) {
            kprintf("\tYour system does not support MultiProcessor Specification!\n");
            return 0;
        }
    }

    return 1;
}

void init_mps()
{
    kprintf("Initializing Intel MultiProcessor Specification 1.4\n");
    
    // Search for MP Floating Pointer Structure
    int found_fps = find_fps();
    if (!found_fps) {
        kprintf("\tUnable to find MP floating pointer!\n");
        return;
    }
    
    // Search for MP Configuration Table or construct a default table
    int found_ct = find_config_table();
    if (!found_ct) {
        kprintf("\tUnable to find MP configuration table!\n");
        return;
    }
    
    // Get information about all processors
    int config_succeed = mps_configure();
    if (!config_succeed) {
        kprintf("\tUnable to parse MP tables!\n");
        return;
    }
    
    // Now we are sure that this system supports MP Specification
    mps_supported = 1;
    
}

