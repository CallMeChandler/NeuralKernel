#pragma once

#include <stdint.h>

namespace pmm
{
    void initialize();

    void* alloc_page();

    void free_page(void* page);

    void print_stats();
}