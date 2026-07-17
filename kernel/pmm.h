#pragma once
#include <stdint.h>

namespace pmm
{
    void initialize();
    void *alloc_page();
    void *alloc_reclaimable_page(uint32_t owner_task, uint8_t size_class = 0);
    void free_page(void *page);
    void touch_page(void *page, bool written = false);
    bool set_reclaimable(void *page, bool reclaimable);
    uint32_t used_pages_count();
    uint32_t free_pages_count();
    uint32_t neural_evictions();
    int32_t last_eviction_score();
    void print_stats();
}
