#pragma once
#include <stdint.h>

namespace nn_pmm
{
    struct Features
    {
        uint32_t recency;
        uint32_t frequency;
        uint32_t allocation_age;
        uint32_t task_affinity;
        uint32_t access_stride;
        bool dirty;
        uint32_t size_class;
        uint32_t recent_fault_rate;
    };

    int32_t score(const Features &features);
}
