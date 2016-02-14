asm (".code16gcc");
asm ("jmp main");


#include "common/include/data.h"
#include "common/include/floppyimg.h"
#include "common/include/memlayout.h"
#include "common/include/bootparam.h"


#define FAT_MASTER_BUFFER_SEGMENT   0
#define FAT_MASTER_BUFFER_OFFSET    BOOT_SECTOR_BUFFER_OFFSET_1
#define FAT_SLAVE_BUFFER_SEGMENT    0
#define FAT_SLAVE_BUFFER_OFFSET     BOOT_SECTOR_BUFFER_OFFSET_2

#define LOADER_FILE_NAME    ((u8 *)"tdlrldr.bin")
#define COREIMG_FILE_NAME   ((u8 *)"tdlrcore.img")

#define MASTER_FAT_LBA      16

#define FAT_MASTER          ((struct floppy_fat_master *)FAT_MASTER_BUFFER_OFFSET)
#define FAT_SLAVE           ((struct floppy_fat_slave *)FAT_SLAVE_BUFFER_OFFSET)


static void real_mode print_char(char ch)
{
    __asm__ __volatile__
    (
        "movb   $0xe, %%ah;"
        "movb   $0, %%bh;"
        "int    $0x10"
        :
        : "a" ((u32)ch)
    );
}

static void real_mode print_string(char *str)
{
    while (*str) {
        print_char(*str);
        str++;
    }
}

static void real_mode print_hex(u32 n)
{
    u32 i, value;
    
    for (i = 0; i < sizeof(u32) * 8; i += 4) {
        value = n;
        value = value << i;
        value = value >> sizeof(u32) * 7;
        
        print_char(value > 9 ? (u8)(value + 0x30 + 0x27) : (u8)(value + 0x30));
    }
    
    print_char('h');
}

static void real_mode print_new_line()
{
    __asm__ __volatile__
    (
        "movw   $0x300, %%ax;"
        "movw   $0, %%bx;"
        "int    $0x10;"
        
        "movw   $0x200, %%ax;"
        "addw   $0x100, %%dx;"
        "movb   $0, %%dl;"
        
        "int    $0x10"
        :
        :
    );
}

/*
 * Return
 *      0 = Same
 *      1 = Different
 */
static int real_mode compare(u8* src, u8* dest, u8 length)
{
    u32 i;
    
    for (i = 0; i < length; i++) {
        if (src[i] != dest[i]) {
            return 1;
        }
    }
    
    return 0;
}

static void real_mode read_sector(u32 lba, u16 segment, u16 offset)
{
    u32 sector = (lba % 18) + 1;
    u32 head = (lba / 18) % 2;
    u32 cylinder = (lba / 18) / 2;
    
    /*
     * AH 02h
     * AL Sectors To Read Count
     */
    u32 eax = 2;
    eax = eax << 8;
    eax += 1;
    
    /*
     * ES:BX   Buffer Address Pointer
     */
    u32 es = segment;
    u32 ebx = offset;
    
    /*
     * DH      Head
     * DL      Drive
     */
    u32 edx = head;
    edx = edx << 8;
    edx += 0;
    
    /*
     * CX =       ---CH--- ---CL---
     * cylinder : 76543210 98
     * sector   :       543210
     */
    u32 ecx = cylinder;
    ecx = ecx << 8;
    ecx = ecx & 0xFFFF;
    ecx += cylinder >> 2;
    ecx = ecx & 0xFFFFFFC0;
    ecx += sector;
    
    /*
     * Make a BIOS call
     */
    __asm__ __volatile__
    (
        "movw   %%ax, %%es;"
        
        "_try_again:"
        "movb   $0x2, %%ah;"
        "movb   $0x1, %%al;"
        "int    $0x13;"
        
        "jc     _try_again;"
        :
        : "a" (es), "b" (ebx), "c" (ecx), "d" (edx)
    );
}

#define load_to_fat_master_buffer(lba) read_sector(lba, FAT_MASTER_BUFFER_SEGMENT, FAT_MASTER_BUFFER_OFFSET);
#define load_to_fat_slave_buffer(lba) read_sector(lba, FAT_SLAVE_BUFFER_SEGMENT, FAT_SLAVE_BUFFER_OFFSET);

static void real_mode load_file(u16 start_sector, u16 sector_count, u16 segment, u16 offset)
{
    u32 current_segment = segment;
    u32 current_offset = offset;
    
    u32 current_lba = start_sector;
    u32 i;
    
    for (i = 0; i < sector_count; i++) {
        print_char('.');
        
        current_lba = start_sector + i;
        
        // Check whether we need to move to the next segment
        if (current_offset >= 65536) {
            print_char('|');
            
            current_segment += 0x1000;
            current_offset = 0;
        }
        
        read_sector(current_lba, current_segment, current_offset);
        
        current_offset += 512;
    }
}

static void real_mode stop()
{
    print_new_line();
    print_string("Unable to load Toddler!");
    print_new_line();
    
    do {
        __asm__ __volatile__
        (
            "hlt"
            :
            :
        );
    } while (1);
}

static void real_mode setup_bootdev()
{
    struct boot_param *bp = (struct boot_param *)BOOT_PARAM_ADDRESS_OFFSET;
    
    bp->boot_dev = 1;
    bp->boot_dev_info = 0;
}

static void real_mode jump_to_loader()
{
    print_string("Starting OS Loader ...");
    
    __asm__ __volatile__
    (
        "movw   %%ax, %%ds;"
        "jmpl   %0, %1;"
        :
        : "n" (LOADER_LOAD_SEGMENT), "n" (LOADER_LOAD_OFFSET),
          "a" (LOADER_LOAD_SEGMENT)
    );
}

static void real_mode parse_master_fat()
{
    print_string("Loading Master FAT ... ");
    
    // Read the master FAT
    load_to_fat_master_buffer(MASTER_FAT_LBA);
    
    print_string("FAT Count: ");
    print_hex(FAT_MASTER->header.fat_count);
    
    print_string(", File Count: ");
    print_hex(FAT_MASTER->header.file_count);
    
    print_new_line();
}

static int real_mode find_and_load_file(u8* file_name, u16 segment, u16 offset)
{
    u16 fat_count = 0;
    u16 file_count = 0;
    
    u16 current_fat = 0;
    u16 current_file = 0;
    
    u16 entry_count;
    u32 i;
    
    struct floppy_fat_entry *current_entry;
    
    fat_count = FAT_MASTER->header.fat_count;
    file_count = FAT_MASTER->header.file_count;
    
    // For every slave FAT
    for (current_fat = 0; current_fat < fat_count; current_fat++) {
        // If this is a slave FAT, load it
        if (current_fat) {
            load_to_fat_slave_buffer(MASTER_FAT_LBA + current_fat);
        }
        
        // Master FAT has 31 entries, each slave FAT has 32 entries
        if (current_fat) {
            entry_count = 32;
        } else {
            entry_count = 31;
        }
        
        // For every entry in this FAT
        for (i = 0; i < entry_count; i++) {
            // Check whether there is no more file
            if (current_file >= file_count) {
                break;
            }
            
            // Get current entry
            if (current_fat) {
                current_entry = &(FAT_SLAVE->entries[i]);
            } else {
                current_entry = &(FAT_MASTER->entries[i]);
            }
            
            // Compare file name
            if (!compare(current_entry->file_name, file_name, 12)) {
                print_string((char *)file_name);
                
                // Load the file
                load_file(current_entry->start_sector, current_entry->sector_count, segment, offset);
                
                print_new_line();
                
                return 1;
            }
            
            // Did not find
            current_file++;
        }
    }
    
    // Did not find the file
    return 0;
}

#define find_and_load_loader()  find_and_load_file(LOADER_FILE_NAME, LOADER_LOAD_SEGMENT, LOADER_LOAD_OFFSET)
#define find_and_load_core()    find_and_load_file(COREIMG_FILE_NAME, COREIMG_LOAD_SEGMENT, COREIMG_LOAD_OFFSET)

int main()
{
    // First we parse master FAT
    parse_master_fat();
    
    // Find and load loader and core image
    int loader = find_and_load_loader();
    int core = find_and_load_core();
    
    // If both loader and kernel are loaded, jump to loader
    if (loader && core) {
        setup_bootdev();
        jump_to_loader();
    }
    
    // Stop
    stop();
    
    // Just to make GCC happy
    return 0;
}
