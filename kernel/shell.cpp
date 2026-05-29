#include "shell.h"
#include "terminal.h"
#include "printk.h"
#include "telemetry.h"
#include "pmm.h"

namespace shell
{
    static char input_buffer[256];

    static int buffer_index = 0;

    static void print_prompt()
    {
        terminal::write("NeuralKernel> ");
    }

    static bool strcmp(
        const char *a,
        const char *b)
    {
        int i = 0;

        while (a[i] && b[i])
        {
            if (a[i] != b[i])
            {
                return false;
            }

            i++;
        }

        return a[i] == b[i];
    }

    static void execute_command()
    {
        telemetry::commands_executed++;
        terminal::putchar('\n');

        if (strcmp(input_buffer, "help"))
        {
            terminal::write(
                "Commands:\n");

            terminal::write(
                "help\n");

            terminal::write(
                "clear\n");

            terminal::write(
                "about\n");

            terminal::write(
                "telemetry\n");

            terminal::write(
                "pmm\n");
        }

        else if (strcmp(input_buffer, "clear"))
        {
            terminal::clear();
        }

        else if (strcmp(input_buffer, "about"))
        {
            terminal::write(
                "NeuralKernel Experimental AI OS\n");
        }

        else if (strcmp(input_buffer, "telemetry"))
        {
            telemetry::print();
        }

        else if (strcmp(input_buffer, "pmm"))
        {
            pmm::print_stats();
        }

        else if (buffer_index > 0)
        {
            terminal::write(
                "Unknown command: ");

            terminal::write(
                input_buffer);

            terminal::putchar('\n');
        }

        buffer_index = 0;

        input_buffer[0] = '\0';

        print_prompt();
    }

    void initialize()
    {
        print_prompt();
    }

    void handle_input(char c)
    {

        // enter
        if (c == '\n')
        {
            input_buffer[buffer_index] = '\0';
            execute_command();
            return;
        }

        // backspace
        if (c == '\b')
        {
            if (buffer_index > 0)
            {
                buffer_index--;

                terminal::putchar('\b');
            }

            return;
        }

        // normal chars
        if (buffer_index < 255)
        {
            input_buffer[buffer_index] = c;

            buffer_index++;

            terminal::putchar(c);
        }
    }

}