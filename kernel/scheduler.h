#pragma once

#include <stdint.h>

namespace scheduler
{
    void initialize();

    void run();

    void tick();

    bool should_schedule();

    uint32_t get_ticks();
}