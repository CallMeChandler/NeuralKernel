#pragma once

#include <stdint.h>

namespace task
{
    typedef void (*TaskFunction)();

    struct Task
    {
        uint32_t id;
        bool active;
        TaskFunction function;
    };

    void initialize();

    int create(TaskFunction function);

    Task* get_tasks();

    int get_task_count();
}