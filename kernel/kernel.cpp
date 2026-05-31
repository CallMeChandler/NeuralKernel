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
#include "paging.h"
#include "heap.h"
#include "scheduler.h"
#include "task.h"
#include "context.h"

void taskA()
{
    while (true)
    {
        terminal::write("A");

        if (scheduler::should_schedule())
        {
            scheduler::run();
        }
    }
}

void taskB()
{
    while (true)
    {
        terminal::write("B");

        if (scheduler::should_schedule())
        {
            scheduler::run();
        }
    }
}

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

    paging::initialize();

    void *page1 =
        pmm::alloc_page();

    void *page2 =
        pmm::alloc_page();

    heap::initialize();

    void *a = heap::kmalloc(64);

    void *b = heap::kmalloc(128);

    shell::initialize();

    task::initialize();

    scheduler::initialize();

    task::create(taskA);
    task::create(taskB);

    __asm__("sti");

    printk::log(
        printk::INFO,
        "Interrupts enabled");

    scheduler::run();

    while (1)
    {
        asm volatile("hlt");
    }
}