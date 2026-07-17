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
#include "syscall.h"
#include "vfs.h"
#include "elf.h"
#include "auth.h"
#include "splash.h"
#include "nn_scheduler.h"
#include "watchdog.h"

void idle_task()
{
    while (true)
    {
        asm volatile("hlt");

        watchdog::poll();

        if (scheduler::should_schedule())
        {
            task::yield();
        }
    }
}

void neural_interactive_demo()
{
    while (true)
        task::sleep(20);
}

void neural_cpu_demo()
{
    volatile uint32_t accumulator = 0;
    while (true)
    {
        for (uint32_t i = 0; i < 20000; i++)
            accumulator = accumulator * 33U + i;
        task::yield();
    }
}

void worker1()
{
    while (true)
    {
        terminal::write("1");

        task::sleep(50);
    }
}

void worker2()
{
    while (true)
    {
        terminal::write("2");

        task::sleep(100);
    }
}

void syscall_demo()
{
    while (true)
    {
        syscall::handle(
            0,
            (uint32_t)"SYS",
            0,
            0);

        syscall::handle(
            1,
            100,
            0,
            0);
    }
}

extern "C" uint8_t _binary_initrd_nkfs_start[];
extern "C" uint8_t _binary_initrd_nkfs_end[];

static void mount_initrd()
{
    uint32_t initrd_size =
        (uint32_t)(_binary_initrd_nkfs_end - _binary_initrd_nkfs_start);
    vfs::init(_binary_initrd_nkfs_start, initrd_size);
}

static void load_user_program()
{
    uint32_t elf_size = 0;
    const void *image = vfs::open("hello.elf", &elf_size);

    if (image == nullptr)
    {
        printk::log(printk::ERROR, "hello.elf not found");
        return;
    }

    uint32_t entry = 0;

    if (!elf::load(image, elf_size, &entry))
    {
        printk::log(printk::ERROR, "ELF load failed");
        return;
    }

    void *user_stack = pmm::alloc_page();

    if (user_stack == nullptr ||
        !paging::make_user_accessible((uint32_t)user_stack, 4096))
    {
        printk::log(printk::ERROR, "User stack allocation failed");
        return;
    }

    uint32_t user_stack_top = (uint32_t)user_stack + 4096;

    if (task::create_user("hello.elf", entry, user_stack_top) < 0)
    {
        printk::log(printk::ERROR, "User task creation failed");
        return;
    }

    printk::log(printk::INFO, "ELF user task loaded");
}

extern "C" void kernel_main()
{
    terminal::initialize();
    splash::show();

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

    mount_initrd();
    auth::initialize();

    task::initialize();

    nn_scheduler::initialize();
    scheduler::initialize();
    watchdog::initialize();

    syscall::initialize();

    // Task 0 = idle task
    task::create(
        "idle",
        idle_task);

    // Silent demo workloads: no shell pollution, but enough behavior for
    // the neural classifier and watchdog to observe after login.
    task::create("interactive", neural_interactive_demo);
    task::create("cpu-demo", neural_cpu_demo);

    load_user_program();

    __asm__("sti");

    printk::log(
        printk::INFO,
        "Interrupts enabled");

    auth::login();
    scheduler::schedule();

    while (true)
    {
        asm volatile("hlt");
    }
}