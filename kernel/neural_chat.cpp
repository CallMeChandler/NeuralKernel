#include "neural_chat.h"
#include "nlp.h"
#include "terminal.h"
#include "auth.h"
#include "vfs.h"
#include "pmm.h"
#include "heap.h"
#include "telemetry.h"
#include "task.h"
#include "nn_scheduler.h"
#include "io.h"
#include <stdint.h>

namespace neural_chat
{
    static bool enabled = false;
    static bool reboot_pending = false;

    static bool equals(const char *left, const char *right)
    {
        uint16_t i = 0;
        while (left && right && left[i] && right[i])
        {
            char a = left[i], b = right[i];
            if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
            if (b >= 'A' && b <= 'Z') b = b - 'A' + 'a';
            if (a != b) return false;
            i++;
        }
        return left && right && left[i] == right[i];
    }

    static bool filler(const char *word)
    {
        return equals(word, "open") || equals(word, "read") || equals(word, "show") ||
               equals(word, "display") || equals(word, "cat") || equals(word, "file") ||
               equals(word, "the") || equals(word, "my") || equals(word, "please") ||
               equals(word, "me") || equals(word, "that");
    }

    static void print_number(uint32_t value)
    {
        if (!value) { terminal::putchar('0'); return; }
        char digits[16]; uint16_t count = 0;
        while (value) { digits[count++] = '0' + value % 10; value /= 10; }
        while (count) terminal::putchar(digits[--count]);
    }

    static uint32_t active_tasks()
    {
        uint32_t count = 0;
        for (int i = 0; i < task::get_task_count(); i++)
        {
            task::Task *entry = task::get_task(i);
            if (entry && entry->active && entry->state != task::TaskState::FINISHED) count++;
        }
        return count;
    }

    static bool find_filename(const char *line, char *output, uint16_t capacity)
    {
        char word[64];
        uint16_t length = 0;
        for (uint16_t i = 0;; i++)
        {
            char c = line[i];
            bool separator = c == 0 || c == ' ' || c == '\t' || c == ',' || c == '?' || c == '!';
            if (!separator && length < sizeof(word) - 1)
            {
                if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
                word[length++] = c;
            }
            if (separator && length)
            {
                word[length] = 0;
                if (!filler(word))
                {
                    uint16_t j = 0;
                    while (word[j] && j + 1 < capacity) { output[j] = word[j]; j++; }
                    output[j] = 0;
                    if (vfs::valid_nkfs_name(output) && vfs::exists(output)) return true;
                    if (!vfs::valid_nkfs_name(output) && j + 6 < capacity)
                    {
                        output[j++] = '.'; output[j++] = 'n'; output[j++] = 'k';
                        output[j++] = 'f'; output[j++] = 's'; output[j] = 0;
                        if (vfs::exists(output)) return true;
                    }
                }
                length = 0;
            }
            if (!c) break;
        }
        output[0] = 0;
        return false;
    }

    static void show_memory()
    {
        uint32_t used = pmm::used_pages_count();
        uint32_t free = pmm::free_pages_count();
        uint32_t total = used + free;
        terminal::write("Memory looks ");
        if (total && free * 100 / total > 50) terminal::write("healthy.\n");
        else if (total && free * 100 / total > 20) terminal::write("moderately loaded.\n");
        else terminal::write("under pressure.\n");
        terminal::write("  Physical pages: "); print_number(used);
        terminal::write(" used, "); print_number(free); terminal::write(" free.\n");
        terminal::write("  Heap: "); print_number(heap::allocated_bytes());
        terminal::write(" bytes used, "); print_number(heap::free_bytes());
        terminal::write(" bytes free.\n");
    }

    static void show_tasks()
    {
        uint32_t count = active_tasks();
        terminal::write("I found "); print_number(count); terminal::write(" active tasks:\n");
        for (int i = 0; i < task::get_task_count(); i++)
        {
            task::Task *entry = task::get_task(i);
            if (!entry || !entry->active || entry->state == task::TaskState::FINISHED) continue;
            terminal::write("  "); terminal::write(entry->name);
            terminal::write(" ["); terminal::write(task::state_string(entry->state)); terminal::write("]");
            terminal::write(" - "); terminal::write(nn_scheduler::class_name(nn_scheduler::class_for(i)));
            terminal::putchar('\n');
        }
    }

    void enter()
    {
        enabled = true;
        reboot_pending = false;
        terminal::write("Neural conversational mode enabled.\n");
        terminal::write("Ask about files, memory, tasks, uptime or identity.\n");
        terminal::write("Type 'exit neural' to return.\n");
    }

    void leave()
    {
        enabled = false;
        reboot_pending = false;
    }

    bool active() { return enabled; }

    void handle_line(const char *line)
    {
        if (reboot_pending)
        {
            if (equals(line, "confirm reboot"))
            {
                terminal::write("Rebooting NeuralKernel...\n");
                outb(0x64, 0xFE);
                for (;;) asm volatile("hlt");
            }
            reboot_pending = false;
            terminal::write("Reboot cancelled.\n");
            return;
        }

        nlp::Result result = nlp::classify(line);
        switch (result.intent)
        {
            case nlp::Intent::GREET:
                terminal::write("Hello, "); terminal::write(auth::username());
                terminal::write(". NeuralKernel is online with ");
                print_number(active_tasks()); terminal::write(" active tasks.\n");
                break;

            case nlp::Intent::LIST_FILES:
                terminal::write("Here is what I found in NKFS:\n");
                vfs::list();
                break;

            case nlp::Intent::OPEN_FILE:
            {
                char filename[64];
                if (!find_filename(line, filename, sizeof(filename)))
                {
                    terminal::write("I need an existing .nkfs filename. Try: open notes.nkfs\n");
                    break;
                }
                uint32_t size = 0;
                const char *data = (const char *)vfs::open(filename, &size);
                terminal::write("Opening "); terminal::write(filename); terminal::write("...\n\n");
                for (uint32_t i = 0; i < size; i++) terminal::putchar(data[i]);
                if (!size || data[size - 1] != '\n') terminal::putchar('\n');
                break;
            }

            case nlp::Intent::MEMORY_INFO:
                show_memory();
                break;

            case nlp::Intent::TASK_INFO:
                show_tasks();
                break;

            case nlp::Intent::UPTIME_INFO:
                terminal::write("NeuralKernel has been running for ");
                print_number(telemetry::timer_ticks / 100);
                terminal::write(" seconds.\n");
                break;

            case nlp::Intent::WHOAMI:
                terminal::write("You are logged in as ");
                terminal::write(auth::username()); terminal::write(".\n");
                break;

            case nlp::Intent::HELP:
                terminal::write("I can list or open files, inspect memory, show tasks, report uptime, identify the user, and request a reboot.\n");
                terminal::write("Examples: 'show my files', 'open notes', 'how is memory', 'what is running'.\n");
                break;

            case nlp::Intent::REBOOT:
                reboot_pending = true;
                terminal::write("Reboot the machine? Type 'confirm reboot' to continue.\n");
                break;

            case nlp::Intent::EXIT_MODE:
                leave();
                terminal::write("Returning to the standard shell.\n");
                break;

            default:
                terminal::write("I do not understand that request yet.\n");
                terminal::write("I can inspect files, memory, tasks, uptime and system identity.\n");
                break;
        }
    }
}
