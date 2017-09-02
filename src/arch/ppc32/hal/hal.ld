ENTRY(hal_entry)

SECTIONS
{
    . = 0xFFF88000;
    
    __start = .;
    
    __text_start = .;
    .text : {
        *(entry);
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
    .bss  : {
        *(.bss)
    }
    __bss_end = .;
    
    __end = .;
}
