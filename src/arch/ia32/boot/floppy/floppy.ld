SECTIONS
{
    . = 0x7e00;

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
