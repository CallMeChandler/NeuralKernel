BITS 32

global idt_load
global isr0

extern divide_by_zero

idt_load:
    mov eax,[esp+4]

    lidt [eax]

    ret



isr0:

    cli

    call divide_by_zero

.loop:
    hlt
    jmp .loop