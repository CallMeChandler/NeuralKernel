#include "telemetry.h"

#include "terminal.h"

namespace telemetry
{
    uint32_t timer_ticks = 0;

    uint32_t keyboard_irqs = 0;

    uint32_t keys_pressed = 0;

    uint32_t commands_executed = 0;

    static void print_number(uint32_t n)
    {
        if(n == 0)
        {
            terminal::putchar('0');
            return;
        }

        char buffer[16];

        int i = 0;

        while(n > 0)
        {
            buffer[i] =
                '0' + (n % 10);

            n /= 10;

            i++;
        }

        for(int j = i - 1; j >= 0; j--)
        {
            terminal::putchar(
                buffer[j]
            );
        }
    }

    void initialize()
    {

    }



    void print()
    {
        terminal::write(
            "Telemetry:\n"
        );



        terminal::write(
            "Timer ticks: "
        );

        print_number(
            timer_ticks
        );

        terminal::putchar('\n');



        terminal::write(
            "Keyboard IRQs: "
        );

        print_number(
            keyboard_irqs
        );

        terminal::putchar('\n');



        terminal::write(
            "Keys pressed: "
        );

        print_number(
            keys_pressed
        );

        terminal::putchar('\n');



        terminal::write(
            "Commands executed: "
        );

        print_number(
            commands_executed
        );

        terminal::putchar('\n');
    }
}