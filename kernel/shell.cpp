#include "shell.h"
#include "terminal.h"
#include "telemetry.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "scheduler.h"
#include "vfs.h"
#include "auth.h"
#include "editor.h"
#include "io.h"
#include "nn_scheduler.h"
#include "watchdog.h"
#include "neural_chat.h"

namespace shell
{
    static char buffer[256];
    static int length = 0;

    static bool equals(const char *left, const char *right)
    {
        int i = 0;
        while (left[i] && right[i])
        {
            if (left[i] != right[i]) return false;
            i++;
        }
        return left[i] == right[i];
    }

    static bool starts_with(const char *text, const char *prefix)
    {
        int i = 0;
        while (prefix[i])
        {
            if (text[i] != prefix[i]) return false;
            i++;
        }
        return true;
    }

    static void print_number(uint32_t value)
    {
        if (!value) { terminal::putchar('0'); return; }
        char digits[16]; int count = 0;
        while (value) { digits[count++] = '0' + value % 10; value /= 10; }
        while (count--) terminal::putchar(digits[count]);
    }

    static void print_signed(int32_t value)
    {
        if (value < 0)
        {
            terminal::putchar('-');
            print_number((uint32_t)-value);
        }
        else print_number((uint32_t)value);
    }

    static void prompt()
    {
        terminal::set_color(terminal::LIGHT_CYAN);
        terminal::write(neural_chat::active() ? "NK-AI> " : "NeuralKernel> ");
        terminal::set_color(terminal::LIGHT_GREY);
    }

    void initialize()
    {
        length = 0;
        buffer[0] = 0;
        prompt();
    }

    static void command()
    {
        telemetry::commands_executed++;
        terminal::putchar('\n');

        if (neural_chat::active())
        {
            neural_chat::handle_line(buffer);
            length = 0;
            buffer[0] = 0;
            prompt();
            return;
        }

        if (equals(buffer, "help"))
            terminal::write("help clear about telemetry pmm heap ps neural neuralstats neural-on neural-off ls whoami uptime meminfo uname echo cat write rm edit exit reboot\n");
        else if (equals(buffer, "clear")) terminal::clear();
        else if (equals(buffer, "about")) terminal::write("NeuralKernel Experimental AI OS\n");
        else if (equals(buffer, "telemetry")) telemetry::print();
        else if (equals(buffer, "pmm")) pmm::print_stats();
        else if (equals(buffer, "heap")) heap::print_stats();
        else if (equals(buffer, "ls")) vfs::list();
        else if (equals(buffer, "whoami")) { terminal::write(auth::username()); terminal::putchar('\n'); }
        else if (equals(buffer, "uptime"))
        {
            uint32_t ticks = telemetry::timer_ticks;
            print_number(ticks / 100);
            terminal::write(" seconds (ticks: ");
            print_number(ticks);
            terminal::write(")\n");
        }
        else if (equals(buffer, "meminfo"))
        {
            terminal::write("Memory information\n  PMM used pages: ");
            print_number(pmm::used_pages_count());
            terminal::write("\n  PMM free pages: ");
            print_number(pmm::free_pages_count());
            terminal::write("\n  Heap allocated: ");
            print_number(heap::allocated_bytes());
            terminal::write(" bytes\n  Heap free: ");
            print_number(heap::free_bytes());
            terminal::write(" bytes\n");
        }
        else if (equals(buffer, "uname")) terminal::write("NeuralKernel v0.1 i386\n");
        else if (starts_with(buffer, "echo ")) { terminal::write(buffer + 5); terminal::putchar('\n'); }
        else if (starts_with(buffer, "cat "))
        {
            const char *file = buffer + 4;
            if (!vfs::valid_nkfs_name(file)) terminal::write("Warning: cat accepts .nkfs files only.\n");
            else
            {
                uint32_t size = 0;
                const char *data = (const char *)vfs::open(file, &size);
                if (!data) terminal::write("File not found.\n");
                else
                {
                    for (uint32_t i = 0; i < size; i++) terminal::putchar(data[i]);
                    if (!size || data[size - 1] != '\n') terminal::putchar('\n');
                }
            }
        }
        else if (starts_with(buffer, "write "))
        {
            char *file = buffer + 6;
            char *text = file;
            while (*text && *text != ' ') text++;
            if (!*text) terminal::write("Usage: write file.nkfs text\n");
            else
            {
                *text++ = 0;
                if (*text == '"')
                {
                    text++;
                    int i = 0;
                    while (text[i] && text[i] != '"') i++;
                    text[i] = 0;
                }
                int text_length = 0;
                while (text[text_length]) text_length++;
                terminal::write(vfs::write(file, text, text_length)
                    ? "Written.\n"
                    : "Write failed. Use a .nkfs filename.\n");
            }
        }
        else if (starts_with(buffer, "rm "))
            terminal::write(vfs::remove(buffer + 3)
                ? "Removed.\n"
                : "Remove failed. Use a .nkfs filename.\n");
        else if (starts_with(buffer, "edit "))
        {
            if (editor::open(buffer + 5)) { length = 0; return; }
        }
        else if (equals(buffer, "neural"))
            neural_chat::enter();
        else if (equals(buffer, "neural-on"))
        {
            nn_scheduler::set_enabled(true);
            watchdog::set_enabled(true);
            terminal::write("Neural scheduling and watchdog enabled.\n");
        }
        else if (equals(buffer, "neural-off"))
        {
            nn_scheduler::set_enabled(false);
            watchdog::set_enabled(false);
            terminal::write("Neural scheduling and watchdog disabled.\n");
        }
        else if (equals(buffer, "neuralstats"))
        {
            terminal::write("Neural runtime\n  Enabled: ");
            terminal::write(nn_scheduler::enabled() ? "yes" : "no");
            terminal::write("\n  Scheduler decisions: ");
            print_number(scheduler::neural_decisions());
            terminal::write("\n  PMM neural evictions: ");
            print_number(pmm::neural_evictions());
            terminal::write("\n  Watchdog score: ");
            print_number(watchdog::last_score());
            terminal::write("\n  Watchdog warnings: ");
            print_number(watchdog::warnings());
            terminal::write("\n  Interventions: ");
            print_number(watchdog::interventions());
            terminal::putchar('\n');
        }
        else if (equals(buffer, "exit")) { length = 0; auth::logout(); return; }
        else if (equals(buffer, "reboot"))
        {
            terminal::write("Rebooting...\n");
            outb(0x64, 0xFE);
            for (;;) asm volatile("hlt");
        }
        else if (equals(buffer, "ps"))
        {
            terminal::write("ID STATE CLASS SCORE NAME\n");
            for (int i = 0; i < task::get_task_count(); i++)
            {
                task::Task *current = task::get_task(i);
                if (!current || !current->active) continue;
                print_number(current->id);
                terminal::write(" ");
                terminal::write(task::state_string(current->state));
                terminal::write(" ");
                terminal::write(nn_scheduler::class_name(nn_scheduler::class_for(i)));
                terminal::write(" ");
                print_signed(nn_scheduler::score_for(i));
                terminal::write(" ");
                terminal::write(current->name);
                terminal::putchar('\n');
            }
        }
        else if (length)
        {
            terminal::write("Unknown command: ");
            terminal::write(buffer);
            terminal::putchar('\n');
        }

        length = 0;
        buffer[0] = 0;
        prompt();
    }

    void handle_input(char c)
    {
        if (c == '\n')
        {
            buffer[length] = 0;
            command();
            return;
        }
        if (c == '\b')
        {
            if (length) { length--; terminal::putchar('\b'); }
            return;
        }
        if (length < 255)
        {
            buffer[length++] = c;
            terminal::putchar(c);
        }
    }
}
