asm (".code16gcc");
asm ("jmp main");


#include "loader/loader.h"
#include "common/include/data.h"
#include "common/include/memlayout.h"


static struct loader_variables *loader_var;


void real_mode set_cursor_pos(u32 row, u32 col)
{
    u32 edx = (row << 8) + col;
    
    __asm__ __volatile__
    (
        "push   %%edx;"
        "movw   $0x300, %%ax;"
        "movw   $0, %%bx;"
        "int    $0x10;"
        "movw   $0x200, %%ax;"
        "pop    %%edx;"
        "int    $0x10"
        :
        : "d" (edx)
    );
}

void real_mode print_new_line()
{
    loader_var->cursor_row++;
    loader_var->cursor_col = 0;
    set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
}

void real_mode print_char(char ch)
{
    __asm__ __volatile__
    (
        "movb   $0x9, %%ah;"
        "movw   $0x7, %%bx;"
        "movw   $0x1, %%cx;"
        "int    $0x10"
        :
        : "a" ((u32)ch)
    );
    
    loader_var->cursor_col++;
    if (80 == loader_var->cursor_col) {
        print_new_line();
    } else {
        set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
    }
}

void real_mode print_string(char *str)
{
    while (*str) {
        switch (*str) {
        case '\n':
        case '\r':
            print_new_line();
            break;
        case '\\':
            print_char('\\');
            break;
        case '\t':
            loader_var->cursor_col /= 8;
            loader_var->cursor_col = (loader_var->cursor_col + 1) * 8;
            if (loader_var->cursor_col >= 80) {
                print_new_line();
            } else {
                set_cursor_pos(loader_var->cursor_row, loader_var->cursor_col);
            }
            break;
        default:
            print_char(*str);
            break;
        }
        
        str++;
    }
}

void real_mode print_bin(u32 n)
{
    u32 i, value;
    
    for (i = 0; i < sizeof(u32) * 8; i++) {
        value = n;
        value = value << i;
        value = value >> sizeof(u32) * 8 - 1;
        
        print_char(value ? '1' : '0');
    }
    
    print_char('b');
}

void real_mode print_dec(u32 n)
{
    u32 i;
    u8 s[12];
    
    for (i = 11; i >= 0; i--) {
        s[i] = 0;
    }
    
    i = 0;
    do  {
        s[i++] = n % 10 + '0';
    } while ((n /= 10 ) >  0);
    
    for (i = 11; i >= 0; i--) {
        if (s[i]) {
            print_char(s[i]);
        }
    }
}

void real_mode print_hex(u32 n)
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

void real_mode stop()
{
    print_string("\nUnable to start Toddler!\n");
    
    do {
        __asm__ __volatile__
        (
            "hlt"
            :
            :
        );
    } while (1);
}

void real_mode print_done()
{
    print_string(" Done!\n");
}

void real_mode print_failed()
{
    print_string(" Failed!\n");
    stop();
}

void real_mode init_cursor_pos()
{
    u32     edx;
    
    __asm__ __volatile__
    (
        "xorl   %%edx, %%edx;"
        "movw   $0x300, %%ax;"
        "movw   $0, %%bx;"
        "int    $0x10"
        : "=d" (edx)
        :
        : "%%eax", "%%ebx"
    );
    
    loader_var->cursor_row = ((edx << 16) >> 24);
    loader_var->cursor_col = ((edx << 24) >> 24);
    
    print_string(" OS Loader is now running!\n");
}

void real_mode initialize_hardware()
{
    print_string("Initializing Hardwares ...");
    print_done();
}

void real_mode detect_memory_e820()
{
    print_string("Detecting Memory Map By E820 ...");
    
    u32 cur_entry_addr = (u32)loader_var->e820_map;
    loader_var->mem_part_count = 0;
    u32 finish = 0;
    u32 signature;
    
    do
    {
        if (loader_var->mem_part_count >= 128) {
            goto failed;
        }
        
        __asm__ __volatile__
        (
            "int    $0x15"
            : "=a"(signature), "=b"(finish)
            : "a"(0xe820), "b"(finish), "c"(sizeof(struct e820_entry)),
              "d"(0x534d4150), "D"(cur_entry_addr)
        );
        
        if (signature != 0x534d4150) {
            goto failed;
        }
        
        // Continue
        loader_var->mem_part_count++;
        cur_entry_addr += sizeof(struct e820_entry);
        print_char('.');
    } while (finish);
    
    print_done();
    return;
    
failed:
    print_failed();
    return;
}

int real_mode empty_8042()
{
    u8 status;
    u32 loops = 10;
    u32 ffs   = 32;
    
    while (loops--) {
        io_delay();
        
        status = io_in_u8(0x64);
        if (status == 0xff) {
            // FF is a plausible, but very unlikely status
            if (!--ffs) {
                return 1; // Assume no KBC present
            }
        }
        if (status & 1) {
            // Read and discard input data
            io_delay();
            io_in_u8(0x60);
        } else if (!(status & 2)) {
            // Buffers empty, finished!
            return 0;
        }
    }
    
    return 1;
}

int real_mode check_a20_enabled()
{
    u16 addr_low_seg = 0;
    u16 addr_low_offset = 0;
    u16 addr_high_seg = 0xffff;
    u16 addr_high_offset = 0x10;
    
    u8 origin_low;
    u8 origin_high;
    u8 new_low;
    u8 new_high;
    
    for (; addr_high_offset <= 0xff; addr_high_offset++, addr_low_offset++) {
        // Read values from the target addresses
        origin_low = read_u8_seg_offset(addr_low_seg, addr_low_offset);
        origin_high = read_u8_seg_offset(addr_high_seg, addr_high_offset);
        
        // If they are not equal, check the next byte
        if (origin_low != origin_high) {
            continue;
        }
        
        // Try to write a new value to high address
        new_high = origin_high + 1;
        write_u8_seg_offset(addr_high_seg, addr_high_offset, new_high);
        
        // Read the value from the low address */
        new_low = read_u8_seg_offset(addr_low_seg, addr_low_offset);
        
        // If they are equal, A20 is not enabled
        if (new_low == new_high) {
            goto failed;
        }
        
        // Restore the value of high address
        write_u8_seg_offset(addr_high_seg, addr_high_offset, origin_high);
    }
    
    return 1;
    
failed:
    return 0;
}

void real_mode enable_a20()
{
    print_string("Enabling A20 ...");
    
    u8 port;
    
    // The general way to enable A20
    port = io_in_u8(0x92);        /* Configuration port A */
    port |=  0x02;                /* Enable A20 */
    port &= ~0x01;                /* Do not reset machine */
    io_out_u8(port, 0x92);
    
    print_done();
    return;
    
    u32 loops = 256;
    u32 kb_error;
    
    while (loops--) {
        // First, check to see if A20 is already enabled
        if (check_a20_enabled()) {
            goto succeed;
        }
        
        // Next, try the BIOS (INT 0x15, AX=0x2401)
        __asm__ __volatile__
        (
            "movl   $0x2401, %%eax;"
            "xorl   %%ebx, %%ebx;"
            "xorl   %%ecx, %%ecx;"
            "xorl   %%edx, %%edx;"
            "int    $0x15;"
            :
            :
        );
        
        if (check_a20_enabled()) {
            goto succeed;
        }
        
        /* Try enabling A20 through the keyboard controller */
        kb_error = empty_8042();
        
        /* BIOS worked, but with delayed reaction */
        if (check_a20_enabled()) {
            goto succeed;
        }
        
        /* Try enabling A20 through the keyboard controller */
        if (!kb_error) {
            empty_8042();
            
            io_out_u8(0xd1, 0x64);          /* Command write */
            empty_8042();
            
            io_out_u8(0xdf, 0x60);          /* A20 on */
            empty_8042();
            
            io_out_u8(0xff, 0x64);          /* Null command, but UHCI wants it */
            empty_8042();
            
            if (check_a20_enabled()) {
                goto succeed;
            }
        }
        
        /* Finally, try enabling the "fast A20 gate" */
        u8 port_a;
        
        port_a = io_in_u8(0x92);        /* Configuration port A */
        port_a |=  0x02;                /* Enable A20 */
        port_a &= ~0x01;                /* Do not reset machine */
        io_out_u8(port_a, 0x92);
        
        if (check_a20_enabled()) {
            goto succeed;
        }
        
        print_char('.');
    }
    
failed:
    print_failed();
    return;
    
succeed:
    print_done();
}

void real_mode enter_protected_mode()
{
    print_string("Entering Protected Mode ...");
    
    __asm__ __volatile__
    (
        "jmp    *%%ebx"
        :
        : "b"(loader_var->return_address)
    );
}

int real_mode main()
{
    loader_var = (struct loader_variables *)LOADER_VARIABLES_ADDRESS_OFFSET;
    
    init_cursor_pos();
    initialize_hardware();
    detect_memory_e820();
    enable_a20();
    enter_protected_mode();
    
    stop();
    return 0;
}
