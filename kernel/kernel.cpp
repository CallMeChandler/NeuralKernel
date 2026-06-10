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

void idleTask()
{
    while(true)
    {
        asm volatile("hlt");
    }
}

void taskA()
{
    while(true)
    {
        terminal::write("A");

        task::sleep(50);
    }
}

void taskB()
{
    while(true)
    {
        terminal::write("B");

        task::sleep(100);
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

    heap::initialize();

    shell::initialize();

    task::initialize();

    scheduler::initialize();

    // Task 0 = idle task

    task::create(taskA);

    task::create(taskB);

    __asm__("sti");

    printk::log(
        printk::INFO,
        "Interrupts enabled");

    scheduler::run();

    while(true)
    {
        asm volatile("hlt");
    }
}