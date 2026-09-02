ENTRY(kmain)

SECTIONS
{
    . = 0x100000;

    .text : ALIGN(4)
    {
        *(.text)
    }

    .rodata : ALIGN(4)
    {
        *(.rodata)
    }

    .data : ALIGN(4)
    {
        *(.data)
    }

    .bss : ALIGN(4)
    {
        *(COMMON)
        *(.bss)
    }
    
    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.rel*)
        *(.llvm*)
    }
}