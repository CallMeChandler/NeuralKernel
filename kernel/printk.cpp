#include "printk.h"
#include "terminal.h"

namespace printk
{
    static const char* level_strings[] =
    {
        "[INFO] ",
        "[WARN] ",
        "[ERROR] "
    };

    void log(Level level, const char* message)
    {
        terminal::write(level_strings[level]);
        terminal::write(message);
        terminal::write("\n");
    }
}