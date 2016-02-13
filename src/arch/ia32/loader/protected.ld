SECTIONS
{
    . = 0xf000;

    .cseg :
    {
        *(.text);
    }

    .dseg :
    {
        *(rodata);
    }

    /DISCARD/ :
    {
        *(.eh_frame);
    }
}
