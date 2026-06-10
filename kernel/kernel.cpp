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

void idle_task()
{
    while (true)
    {
        asm volatile("hlt");

        if (scheduler::should_schedule())
        {
            task::yield();
        }
    }
}

void worker1()
{
    while(true)
    {
        terminal::write("1");

        task::sleep(50);
    }
}

void worker2()
{
    while(true)
    {
        terminal::write("2");

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
    task::create(
        "idle",
        idle_task);

    task::create(
        "worker1",
        worker1);

    task::create(
        "worker2",
        worker2);

    __asm__("sti");

    printk::log(
        printk::INFO,
        "Interrupts enabled");

    scheduler::schedule();

    while (true)
    {
        asm volatile("hlt");
    }
}