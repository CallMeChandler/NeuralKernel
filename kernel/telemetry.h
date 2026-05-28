#pragma once

#include <stdint.h>

namespace telemetry
{
    extern uint32_t timer_ticks;

    extern uint32_t keyboard_irqs;

    extern uint32_t keys_pressed;

    extern uint32_t commands_executed;



    void initialize();

    void print();
}