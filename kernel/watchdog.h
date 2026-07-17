#pragma once
#include <stdint.h>

namespace watchdog
{
    void initialize();
    void set_enabled(bool value);
    bool enabled();
    void poll();
    uint32_t last_score();
    uint32_t warnings();
    uint32_t interventions();
}
