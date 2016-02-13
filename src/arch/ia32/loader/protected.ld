SECTIONS
{
    . = 0xd000;

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
