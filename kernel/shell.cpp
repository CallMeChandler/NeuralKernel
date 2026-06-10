#include "shell.h"
#include "terminal.h"
#include "printk.h"
#include "telemetry.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "scheduler.h"

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

    static void print_number(uint32_t n)
    {
        if (n == 0)
        {
            terminal::putchar('0');
            return;
        }

        char buffer[16];

        int i = 0;

        while (n > 0)
        {
            buffer[i++] =
                '0' + (n % 10);

            n /= 10;
        }

        for (int j = i - 1;
             j >= 0;
             j--)
        {
            terminal::putchar(
                buffer[j]);
        }
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

            terminal::write(
                "heap\n");

            terminal::write(
                "ps\n");
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

        else if (strcmp(input_buffer, "heap"))
        {
            heap::print_stats();
        }

        else if (strcmp(input_buffer, "ps"))
        {
            terminal::write(
                "\nID  STATE   NAME\n");

            int count =
                task::get_task_count();

            terminal::write("Count: ");
            print_number(count);
            terminal::putchar('\n');

            for (int i = 0;
                 i < count;
                 i++)
            {
                task::Task *t =
                    task::get_task(i);

                print_number(
                    t->id);

                terminal::write("   ");

                terminal::write(
                    task::state_string(
                        t->state));

                terminal::write("   ");

                terminal::write(
                    t->name);

                terminal::putchar('\n');
            }
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