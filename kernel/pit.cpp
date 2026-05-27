#include "pit.h"
#include "io.h"
#include "printk.h"

namespace pit
{
    void init()
    {
        printk::log(
            printk::INFO,
            "Starting PIT..."
        );

        int divisor =
            1193180 / 100;

        outb(
            0x43,
            0x36
        );

        outb(
            0x40,
            divisor & 0xFF
        );

        outb(
            0x40,
            divisor>>8
        );

        printk::log(
            printk::INFO,
            "PIT running at 100Hz"
        );
    }
}