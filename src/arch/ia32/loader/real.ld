SECTIONS
{
    . = 0xb800;

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
