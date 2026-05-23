section .multiboot
align 8

header_start:
    dd 0xE85250D6   ; multiboot2 magic number
    dd 0    ; architecture (i386 protected mode, GRUB handles transition)
    dd header_end - header_start ; header length

    ; checksum (must make total sum zero)
    dd -(0xE85250D6 + 0 + (header_end - header_start))

    ; end tag
    dw 0
    dw 0
    dd 8

header_end: