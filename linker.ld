ENTRY(start)

SECTIONS
{
    . = 1M;

    .text :
    {
        /* Garante que o cabeçalho Multiboot seja a primeira coisa no binário */
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}