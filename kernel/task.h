#pragma once

#include <stdint.h>

namespace task
{
    typedef void (*TaskFunction)();

    enum class TaskState
    {
        READY,
        RUNNING,
        SLEEPING,
        FINISHED
    };

    struct Task
    {
        uint32_t id;
        bool active;
        TaskFunction function;
        uint32_t *stack;
        uint32_t esp;
        TaskState state;
        uint32_t wakeup_tick;
    };

    void initialize();

    int create(TaskFunction function);

    Task *get_tasks();

    int get_task_count();

    void setup_initial_context(Task &task);

    void set_current_task(int id);

    void exit();

    void sleep(uint32_t ticks);

    int current_task();
}