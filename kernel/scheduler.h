#pragma once

#include <stdint.h>

namespace scheduler
{
    void initialize();

    void run();

    void tick();

    bool should_schedule();
}