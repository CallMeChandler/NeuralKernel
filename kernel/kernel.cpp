#include "printk.h"
#include "gdt.h"
#include "terminal.h"

extern "C" void kernel_main()
{
    terminal::initialize();

    printk::log(printk::INFO, "Booting kernel...");
    
    gdt::init();

    printk::log(printk::INFO, "Kernel fully initialized");

    while (1)
        __asm__("hlt");
}