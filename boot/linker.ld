ENTRY(start)

SECTIONS
{
    /* Start placing output at 1 MiB */
    . = 0x00100000;

    /* Code */
    .text :
    {
        *(.text)
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
    }

    /* Initialized data */
    .data :
    {
        *(.data)
    }

    /* Uninitialized data */
    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}
