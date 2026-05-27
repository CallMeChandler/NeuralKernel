#include "terminal.h"
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"

extern "C"
void kernel_main()
{
    terminal::initialize();

    printk::log(
        printk::INFO,
        "Booting..."
    );

    gdt::init();

    idt::init();

    pic::remap();

    pit::init();

    printk::log(
        printk::INFO,
        "Kernel alive"
    );

    while(1)
    {
        __asm__(
            "hlt"
        );
    }
}