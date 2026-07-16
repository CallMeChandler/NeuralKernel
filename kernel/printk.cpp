#include "printk.h"
#include "terminal.h"

namespace printk
{
    static const char* level_strings[] = { "[INFO] ", "[WARN] ", "[ERROR] " };
    static const uint8_t level_colors[] = {
        terminal::LIGHT_GREEN, terminal::LIGHT_BROWN, terminal::LIGHT_RED
    };

    void log(Level level, const char* message)
    {
        uint8_t previous = terminal::get_color();
        terminal::set_color(level_colors[level]);
        terminal::write(level_strings[level]);
        terminal::set_color(terminal::LIGHT_GREY);
        terminal::write(message);
        terminal::putchar('\n');
        terminal::set_color(previous & 0x0F, previous >> 4);
    }
}
