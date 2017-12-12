OUTPUT_FORMAT("binary")
ENTRY(start)

SECTIONS
{
    . = 0x4000;
    
    __start = .;

    __text_start = .;
    .text :
    {
        *(BOOTSTRAP);
        *(.text);
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
    .bss  :
    {
        *(.bss);
    }
    __bss_end = .;
    
    . = ALIGN(4096);
    __initrd_info_start = .;
    .initrd_info :
    {
        /* HdrS */
        LONG(0x48647253);
        LONG(0X0);
        LONG(0X0);
        LONG(0X0);
        
        /* Initrd load addr */
        LONG(0X0);
        /* Initrd size */
        LONG(0x0);
        
        LONG(0x0);
    }
    /*. = ALIGN(4096);*/
    __initrd_info_end = .;
    
    __end = .;
}
