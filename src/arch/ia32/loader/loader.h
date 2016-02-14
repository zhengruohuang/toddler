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
    
    // Cursor
    u32 cursor_row;
    u32 cursor_col;
    
    // Video
    u32 video_type;
    u32 framebuffer_paddr;
    u32 res_x;
    u32 res_y;
    u32 bits_per_pixel;
    u32 bytes_per_line;
} packedstruct;

struct vesa_info {
    u8  VESASignature[4];
    u16 VESAVersion;
    u32 OEMStringPtr;
    u8  Capabilities[4];
    u32 VideoModePtr;
    u16 TotalMemory;
    u16 OemSoftwareRev;
    u32 OemVendorNamePtr;
    u32 OemProductNamePtr;
    u32 OemProductRevPtr;
    u8  Reserved[222];
    u8  OemData[256];
} packedstruct;

struct vesa_mode_info {
    u16 ModeAttributes;
    u8  WinAAttributes;
    u8  WinBAttributes;
    u16 WinGranularity;
    u16 WinSize;
    u16 WinASegment;
    u16 WinBSegment;
    u32 WinFuncPtr;
    u16 BytesPerScanLine;
    u16 XResolution;
    u16 YResolution;
    u8  XCharSize;
    u8  YCharSize;
    u8  NumberOfPlanes;
    u8  BitsPerPixel;
    u8  NumberOfBanks;
    u8  MemoryModel;
    u8  BankSize;
    u8  NumberOfImagePages;
    u8  Reserved_page;
    u8  RedMaskSize;
    u8  RedMaskPos;
    u8  GreenMaskSize;
    u8  GreenMaskPos;
    u8  BlueMaskSize;
    u8  BlueMaskPos;
    u8  ReservedMaskSize;
    u8  ReservedMaskPos;
    u8  DirectColorModeInfo;
    u32 PhysBasePtr;
    u32 OffScreenMemOffset;
    u16 OffScreenMemSize;
    u8  Reserved[206];
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
