#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/mem.h"
#include "hal/include/lib.h"
#include "hal/include/acpi.h"


int fadt_supported = 0;
int acpi_shutdown_supported = 0;

static struct acpi_fadt *acpi_fadt;
static int acpi_shutdown_port = -1;
static int acpi_shutdown_sleep_type = -1;

static u32 decode_len(u8 *ptr, int *numlen)
{
    int num_bytes, i;
    u32 ret;
    
    if (*ptr < 64) {
        if (numlen) {
            *numlen = 1;
        }
        return *ptr;
    }
    
    num_bytes = *ptr >> 6;
    
    if (numlen) {
        *numlen = num_bytes + 1;
    }
    
    ret = *ptr & 0xf;
    ptr++;
    
    for (i = 0; i < num_bytes; i++) {
        ret |= *ptr << (8 * i + 4);
        ptr++;
    }
    
    return ret;
}

static u32 skip_name_str(u8 *ptr, u8 *end)
{
    const u8 *ptr0 = ptr;
    
    while (ptr < end &&(*ptr == '^' || *ptr == '\\')) {
        ptr++;
    }
    
    switch (*ptr) {
    case '.':
        ptr++;
        ptr += 8;
        break;
    case '/':
        ptr++;
        ptr += 1 +(*ptr) * 4;
        break;
    case 0:
        ptr++;
        break;
    default:
        ptr += 4;
        break;
    }
    
    return (u32)(ptr - ptr0);
}

static u32 skip_data_ref_obj(u8 *ptr, u8 *end)
{
    kprintf("\t\tACPI Data Type 0x%h\n", *ptr);
    u8 *ptr0;
    
    switch (*ptr){
    case ACPI_OPCODE_PACKAGE:
    case ACPI_OPCODE_BUFFER:
        return 1 + decode_len(ptr + 1, 0);
    case ACPI_OPCODE_ZERO:
    case ACPI_OPCODE_ONES:
    case ACPI_OPCODE_ONE:
        return 1;
    case ACPI_OPCODE_BYTE_CONST:
        return 2;
    case ACPI_OPCODE_WORD_CONST:
        return 3;
    case ACPI_OPCODE_DWORD_CONST:
        return 5;
    case ACPI_OPCODE_STRING_CONST:
        ptr0 = ptr;
        for (ptr++; ptr < end && *ptr; ptr++) {
        }
        
        if (ptr == end) {
            return 0;
        }
        
        return (u32)(ptr - ptr0 + 1);
    default:
        if (*ptr == '^' || *ptr == '\\' || *ptr == '_' ||(*ptr >= 'A' && *ptr <= 'Z')) {
            return skip_name_str(ptr, end);
        }
        
        kprintf("\t\tUnknown Opcode 0x%h\n", *ptr);
        return 0;
    }
}

static u32 skip_ext_op(u8 *ptr, u8 *end)
{
    const u8 *ptr0 = ptr;
    int add;
    kprintf("\t\tACPI Extended Opcode: 0x%h\n", *ptr);
    
    switch (*ptr)
    {
    case ACPI_EXTOPCODE_MUTEX:
        ptr++;
        ptr += skip_name_str(ptr, end);
        ptr++;
        break;
    case ACPI_EXTOPCODE_OPERATION_REGION:
        ptr++;
        ptr += skip_name_str(ptr, end);
        ptr++;
        ptr += add = skip_data_ref_obj(ptr, end);
        
        if (!add) {
            return 0;
        }
        
        ptr += add = skip_data_ref_obj(ptr, end);
        
        if (!add) {
            return 0;
        }
        
        break;
    case ACPI_EXTOPCODE_FIELD_OP:
    case ACPI_EXTOPCODE_INDEX_FIELD_OP:
        ptr++;
        ptr += decode_len(ptr, 0);
        break;
    default:
        kprintf("\t\tUnexpected Extended Opcode: 0x%h\n", *ptr);
        return 0;
    }
    
    return (u32)(ptr - ptr0);
}

static int get_shutdown_sleep_type(u8 *table, u8 *end)
{
    u8 *ptr, *prev = table;
    int sleep_type = -1;
    
    ptr = table + sizeof(struct acpi_sdt_header);
    while (ptr < end && prev < ptr) {
        int add;
        prev = ptr;
        kprintf("\t\tACPI Opcode 0x%h\n", *ptr);
        //kprintf("\t\tACPI Tell %h\n", (unsigned)(ptr - table));
        
        switch (*ptr) {
        case ACPI_OPCODE_EXTOP:
            ptr++;
            ptr += add = skip_ext_op(ptr, end);
            if (!add) {
                return -1;
            }
            break;
        case ACPI_OPCODE_CREATE_WORD_FIELD:
        case ACPI_OPCODE_CREATE_BYTE_FIELD:
            ptr += 5;
            ptr += add = skip_data_ref_obj(ptr, end);
            if (!add)
                return -1;
            ptr += 4;
            break;
        case ACPI_OPCODE_NAME:
            ptr++;
            if (memcmp(ptr, "_S5_", 4) == 0 || memcmp(ptr, "\\_S5_", 4) == 0) {
                int ll;
                u8 *ptr2 = ptr;
                kprintf("\t\tACPI S5 Found\n");
                ptr2 += skip_name_str(ptr, end);
                if (*ptr2 != 0x12) {
                    kprintf("\t\tUnknown Opcode in _S5: 0x%h\n", *ptr2);
                    return -1;
                }
                ptr2++;
                decode_len(ptr2, &ll);
                ptr2 += ll;
                ptr2++;
                
                switch (*ptr2) {
                case ACPI_OPCODE_ZERO:
                    sleep_type = 0;
                    break;
                case ACPI_OPCODE_ONE:
                    sleep_type = 1;
                    break;
                case ACPI_OPCODE_BYTE_CONST:
                    sleep_type = ptr2[1];
                    break;
                default:
                    kprintf("\t\tUnknown data type in _S5: 0x%h\n", *ptr2);
                    return -1;
                }
                
            }
            ptr += add = skip_name_str(ptr, end);
            if (!add) {
                return -1;
            }
            ptr += add = skip_data_ref_obj(ptr, end);
            if (!add) {
                return -1;
            }
            break;
        case ACPI_OPCODE_SCOPE:
        case ACPI_OPCODE_IF:
        case ACPI_OPCODE_METHOD:
            ptr++;
            ptr += decode_len(ptr, NULL);
            break;
        default:
            kprintf("\t\tUnknown opcode 0x%h\n", *ptr);
            return -1;
        }
    }
    
    kprintf("\t\tACPI TYP = %d\n", sleep_type);
    return sleep_type;
}

int init_fadt(struct acpi_fadt *fadt)
{
    acpi_fadt = fadt;
    
    // Map the table to HAL's address space
    kernel_direct_map_array((ulong)fadt, sizeof(struct acpi_fadt), 0);
    kernel_direct_map_array((ulong)fadt, fadt->header.length, 0);
    
    // Checksum
    if (acpi_byte_checksum(fadt, fadt->header.length)) {
        kprintf("\t\tFADT: Checksum Failed!");
        return 0;
    }
    
    // Find and map DSDT
    struct acpi_dsdt *dsdt = (struct acpi_dsdt *)fadt->dsdt_address;
    
    kernel_direct_map_array((long)dsdt, sizeof(struct acpi_dsdt), 0);
    kernel_direct_map_array((ulong)dsdt, dsdt->header.length, 0);
    
    kprintf("\t\tFound DSDT at %p, Len: %d Bytes\n", dsdt, dsdt->header.length);
    
    // ACPI shutdown
    kprintf("\t\tInitializing ACPI Shutdown\n");
    
    acpi_shutdown_port = fadt->pm1a;
    acpi_shutdown_sleep_type = get_shutdown_sleep_type((u8 *)dsdt, (u8 *)((ulong)&dsdt->header + dsdt->header.length));
    if (acpi_shutdown_sleep_type != -1) {
        acpi_shutdown_supported = 1;
    }
    kprintf("\t\t\tACPI shutdown supported: %d, sleep type %d, port 0x%h\n",
            acpi_shutdown_supported, acpi_shutdown_sleep_type, acpi_shutdown_port);
    
    
    //io_out16(acpi_shutdown_port & 0xffff, ACPI_SLP_EN |(acpi_shutdown_sleep_type << ACPI_SLP_TYP_OFFSET));
    
    fadt_supported = 1;
    return 1;
}
