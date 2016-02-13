SECTIONS
{
    . = 0xc400;

    .cseg :
    {
        *(.text);
    }

    .dseg :
    {
        *(rodata);
    }

    .sig : AT(0xcffe)
    {
        SHORT(0x1ffe);
    }

    /DISCARD/ :
    {
        *(.eh_frame);
    }
}
