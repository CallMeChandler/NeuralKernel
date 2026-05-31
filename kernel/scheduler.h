#pragma once

namespace scheduler
{
    void initialize();

    void run();

    void tick();

    bool should_schedule();
}