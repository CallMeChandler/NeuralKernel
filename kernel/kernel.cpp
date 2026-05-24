#include "terminal.h"
#include "printk.h"

extern "C" void kernel_main()
{
    terminal::initialize();

    printk::log(printk::INFO, "Kernel booting...");
    printk::log(printk::INFO, "Terminal initialized");

    printk::log(printk::WARN, "This is a test warning");
    printk::log(printk::ERROR, "This is a test error");

    while (1)
    {
        __asm__("hlt");
    }
}