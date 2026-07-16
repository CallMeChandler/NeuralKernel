#pragma once
#include <stddef.h>
#include <stdint.h>

namespace heap
{
    void initialize();
    void* kmalloc(size_t size);
    void kfree(void* pointer);
    uint32_t allocated_bytes();
    uint32_t free_bytes();
    void print_stats();
}
