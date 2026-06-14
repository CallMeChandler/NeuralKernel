#pragma once

#include <stdint.h>

namespace syscall
{
    void initialize();

    uint32_t handle(
        uint32_t number,
        uint32_t arg1,
        uint32_t arg2,
        uint32_t arg3
    );
}