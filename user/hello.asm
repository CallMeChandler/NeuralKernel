BITS 32

section .text
global _start

_start:
    mov eax, 0
    mov ebx, message
    int 0x80

    mov eax, 2
    xor ebx, ebx
    int 0x80

.hang:
    jmp .hang

section .rodata
message db "[USER] hello.elf is running in Ring 3", 10, 0
