#include "irq.h"
#include "printk.h"
#include "io.h"

namespace irq
{
    static volatile int ticks = 0;

    extern "C"
    void timer_tick()
    {
        ticks++;

        if((ticks % 100)==0)
        {
            printk::log(
                printk::INFO,
                "1 second passed"
            );
        }

        // End Of Interrupt
        outb(
            0x20,
            0x20
        );
    }

    void init()
    {
        printk::log(
            printk::INFO,
            "IRQ system ready"
        );
    }
}