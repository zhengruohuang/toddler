OUTPUT_FORMAT("binary")
ENTRY(start)

SECTIONS
{
    . = 0x01000000;
    __start = .;

    __text_start = .;
    .text : {
        *(BOOTSTRAP);
        *(.text);
    }
    __text_end = .;

    __rodata_start = .;
    .rodata : {
        *(.rodata)
    }
    __rodata_end = .;

    __data_start = .;
    .data : {
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
