#include "irq.h"
#include "printk.h"
#include "io.h"
#include "telemetry.h"
#include "scheduler.h"

namespace irq
{
    static volatile int ticks = 0;

    extern "C" void timer_tick()
    {
        ticks++;

        telemetry::timer_ticks++;

        if ((ticks % 100) == 0)
        {
            // timer is alive
        }

        if ((ticks % 10) == 0)
        {
            scheduler::tick();
        }

        // End Of Interrupt
        outb(
            0x20,
            0x20);
    }

    void init()
    {
        printk::log(
            printk::INFO,
            "IRQ system ready");
    }
}