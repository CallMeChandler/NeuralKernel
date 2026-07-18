#pragma once
#include <stdint.h>

namespace nlp
{
    enum class Intent : uint8_t
    {
        GREET,
        LIST_FILES,
        OPEN_FILE,
        MEMORY_INFO,
        TASK_INFO,
        UPTIME_INFO,
        WHOAMI,
        HELP,
        REBOOT,
        EXIT_MODE,
        UNKNOWN
    };

    struct Result
    {
        Intent intent;
        int16_t score;
        int16_t margin;
    };

    Result classify(const char *text);
    const char *intent_name(Intent intent);
}
