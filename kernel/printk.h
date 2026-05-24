#pragma once

namespace printk
{
    enum Level
    {
        INFO,
        WARN,
        ERROR
    };

    void log(Level level, const char* message);
}