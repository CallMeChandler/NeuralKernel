BITS 32

section .text

global syscall_entry
extern syscall_dispatch

syscall_entry:
    push ds
    push es
    push fs
    push gs
    pushad

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esi, [esp + 28]

    push edx
    push ecx
    push ebx
    push esi

    sti
    call syscall_dispatch
    cli

    add esp, 16
    mov [esp + 28], eax

    popad
    pop gs
    pop fs
    pop es
    pop ds
    iretd
