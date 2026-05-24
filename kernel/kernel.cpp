#include "terminal.h"

extern "C" void kernel_main()
{
    terminal::initialize();

    terminal::set_color(terminal::LIGHT_GREEN, terminal::BLACK);

    terminal::write("NeuralKernel booted successfully\n");
    terminal::write("Terminal subsystem initialized\n");

    while (1)
    {
        __asm__("hlt");
    }
}