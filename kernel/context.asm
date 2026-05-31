global context_switch

section .text

context_switch:

    ; Arguments:
    ; [esp+4] = old_esp pointer
    ; [esp+8] = new_esp

    mov eax, [esp + 4]
    mov edx, [esp + 8]

    ; Save callee-saved registers
    push ebp
    push ebx
    push esi
    push edi

    ; Save current stack pointer
    test eax, eax
    jz .skip_save

    mov [eax], esp

.skip_save:

    ; Load next task stack
    mov esp, edx

    ; Restore next task registers
    pop edi
    pop esi
    pop ebx
    pop ebp

    ret