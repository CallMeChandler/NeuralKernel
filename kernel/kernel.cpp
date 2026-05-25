#include "terminal.h"
#include "printk.h"
#include "gdt.h"
#include "idt.h"

extern "C"
void kernel_main()
{
    terminal::initialize();

    printk::log(
        printk::INFO,
        "Booting kernel..."
    );

    gdt::init();

    idt::init();

    printk::log(
        printk::INFO,
        "Triggering interrupt test"
    );

    __asm__(
        "div %0"
        :
        : "r"(0)
    );

    while(1)
    {
        __asm__("hlt");
    }
}