#include "printk.h"

extern "C"
void divide_by_zero()
{
    printk::log(
        printk::ERROR,
        "CPU Exception: Divide by zero"
    );

    while(1)
    {
        __asm__("hlt");
    }
}