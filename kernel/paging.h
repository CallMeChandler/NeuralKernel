#pragma once

#include <stdint.h>

namespace paging
{
    void initialize();

    bool make_user_accessible(uint32_t start, uint32_t size);
}
