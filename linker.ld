ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(8) :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(16) : { *(.text*) }
    .rodata ALIGN(16) : { *(.rodata*) }
    .data ALIGN(16) : { *(.data*) *(.got*) }
    .bss ALIGN(4096) : { *(COMMON) *(.bss*) }

    /DISCARD/ : { *(.comment*) *(.note*) *(.eh_frame*) }
}
