#pragma once
#include <stddef.h>

namespace heap
{
    void initialize();

    void* kmalloc(size_t size);

    void print_stats();
}