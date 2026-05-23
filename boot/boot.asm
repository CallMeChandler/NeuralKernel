BITS 32

section .text
global _start

extern kernel_main

_start:
    mov esp, stack_top

    call kernel_main

.hang:
    cli

.loop:
    hlt
    jmp .loop


section .bss
align 16
stack_bottom:
    resb 16384
stack_top: