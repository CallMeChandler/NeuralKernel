#pragma once

#include <stdint.h>

extern "C"
{
    void context_switch(
        uint32_t* old_esp,
        uint32_t new_esp
    );
}