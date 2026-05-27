BITS 32

global irq1

extern keyboard_handler

irq1:

    cli

    pushad

    call keyboard_handler

    popad

    sti

    iretd