#include "terminal.h"
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "irq.h"
#include "keyboard.h"
#include "shell.h"
#include "telemetry.h"
#include "pmm.h"

extern "C" void kernel_main()
{
    terminal::initialize();

    printk::log(
        printk::INFO,
        "Booting...");

    gdt::init();

    idt::init();

    pic::remap();

    irq::init();

    pit::init();

    keyboard::init();

    telemetry::initialize();

    pmm::initialize();

    void *page1 =
        pmm::alloc_page();

    void *page2 =
        pmm::alloc_page();

    shell::initialize();

    __asm__("sti");

    printk::log(
        printk::INFO,
        "Interrupts enabled");

    while (1)
    {
        __asm__("hlt");
    }
}