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
        const char *name;
        TaskFunction function;
        uint32_t *stack;
        uint32_t esp;
        uint32_t kernel_stack_top;
        TaskState state;
        uint32_t wakeup_tick;
        bool user_mode;
        uint32_t user_entry;
        uint32_t user_stack_top;
    };

    void initialize();

    int create(
        const char *name,
        TaskFunction function);

    int create_user(
        const char *name,
        uint32_t entry,
        uint32_t user_stack_top);

    Task *get_tasks();
    int get_task_count();
    void setup_initial_context(Task &task);
    void set_current_task(int id);
    void exit();
    void sleep(uint32_t ticks);
    int current_task();
    void yield();
    Task *get_task(int id);
    const char *state_string(TaskState state);
}
