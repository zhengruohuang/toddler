#ifndef __ARCH_IA32_LOADER_HH__
#define __ARCH_IA32_LOADER_HH__


#include "common/include/data.h"


struct e820_entry {
    union {
        struct {
            u32 base_addr_low;
            u32 base_addr_high;
        };
        u64 base_addr;
    };
    
    union {
        struct {
            u32 len_low;
            u32 len_high;
        };
        u64 len;
    };
    
    u32 type;
    u32 reserved;
} packedstruct;

struct loader_variables {
    // Always the first fields in this struct
    u32 return_addr;
    u32 hal_entry_addr;
    u32 what_to_load_addr;
    u32 ap_entry_addr;
    u32 ap_page_dir_pfn_addr;
    u32 bios_invoker_addr;
    
    // Cursor
    u32 cursor_row;
    u32 cursor_col;
    
    // HAL Info
    u32 hal_vaddr_end;
    
    // Memory
    u32 mem_part_count;
    struct e820_entry   e820_map[128];
    union {
        struct {
            u32 mem_size_low;
            u32 mem_size_high;
        };
        u64 mem_size;
    };
} packedstruct;

struct loader_parameters {
    u32 boot_device;
    u32 boot_device_param;
} packedstruct;



static inline void io_out_u8(u8 value, u16 port)
{
    __asm__ __volatile__("outb %0,%1" : : "a" (value), "dN" (port));
}

static inline u8 io_in_u8(u16 port)
{
    u8 v;
    
    __asm__ __volatile__("inb %1,%0" : "=a" (v) : "dN" (port));
    
    return v;
}

static inline void io_delay()
{
    __asm__ __volatile__("outb %%al,%0" : : "dN" (0x80));
}

static inline u8 read_u8_seg_offset(u16 seg, u16 offset)
{
    u8 value;
    
    __asm__ __volatile__
    (
        // Get the current value of DS register
        "xorl   %%edx, %%edx;"
        "movw   %%ds, %%dx;"
        
        // Read the value
        "movw   %%ax, %%ds;"
        "movl   (%%ebx), %%eax;"
        
        // Restore DS
        "movw   %%dx, %%ds;"
        : "=a" (value)
        : "a" (seg), "b" (offset)
        : "%edx"
    );
    
    return value;
}

static inline void write_u8_seg_offset(u16 seg, u16 offset, u8 value)
{
    __asm__ __volatile__
    (
        // Get the current value of DS register
        "xorl   %%edx, %%edx;"
        "movw   %%ds, %%dx;"
        
        // Write the value
        "movw   %%ax, %%ds;"
        "movl   %%edx, (%%ebx);"
        
        //* Restore DS
        "movw   %%dx, %%ds;"
        :
        : "a" (seg), "b" (offset), "c" (value)
        : "%edx"
    );
}


#endif
