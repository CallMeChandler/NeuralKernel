BITS 32

global irq0

extern timer_tick

irq0:

    cli

    pushad

    call timer_tick

    popad

    sti

    iretd