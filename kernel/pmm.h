#pragma once

#include <stdint.h>

namespace pmm
{
    void initialize();

    void* alloc_page();

    void free_page(void* page);

    uint32_t used_pages_count();
    uint32_t free_pages_count();
    void print_stats();
}