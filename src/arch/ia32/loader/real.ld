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
        SHORT(0xcffe);
    }

    /DISCARD/ :
    {
        *(.eh_frame);
    }
}
