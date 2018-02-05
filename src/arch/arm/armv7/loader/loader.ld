ENTRY(_start)


/*
 * QEMU loads the image to 0x10000, while a real RPi 2 load the image to 0x8000
 */
__LOADER_BASE = 0x10000;


SECTIONS
{
    . = __LOADER_BASE;
    __start = .;
    
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    __text_end = .;
 
    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    __rodata_end = .;
 
    __data_start = .;
    .data :
    {
        *(.data)
    }
    __data_end = .;
    
    __bss_start = .;
    .bss :
    {
        *(.bss);
    }
    __bss_end = .;

    /*
     * This makes sure two things
     *  1. The end of loader image is aligned to 4KB boundary
     *  2. The linker does not cut off the zero-byted BSS section from binary
     */
    . = ALIGN(4096) - 1;
    .fill :
    {
        BYTE(0xff);
    }

    __end = .;
}
