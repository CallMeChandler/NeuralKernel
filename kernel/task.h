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

    struct TaskMetrics
    {
        uint32_t total_runtime_ticks;
        uint32_t slices;
        uint32_t sleep_count;
        uint32_t sleep_ticks;
        uint32_t syscall_count;
        uint32_t memory_accesses;
        uint32_t page_faults;
        uint32_t last_run_tick;
        uint32_t last_burst_ticks;
        uint32_t current_burst_ticks;
        uint32_t starvation_ticks;
        uint32_t watchdog_runtime_snapshot;
        uint32_t watchdog_syscall_snapshot;
        uint32_t watchdog_sleep_snapshot;
        uint32_t watchdog_fault_snapshot;
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
        TaskMetrics metrics;
        int8_t dynamic_priority;
        int8_t neural_bias;
        uint8_t mlfq_level;
        uint8_t neural_class;
        uint8_t anomaly_strikes;
        uint32_t last_anomaly_score;
        bool protected_task;
    };

    void initialize();
    int create(const char *name, TaskFunction function);
    int create_user(const char *name, uint32_t entry, uint32_t user_stack_top);
    Task *get_tasks();
    int get_task_count();
    void setup_initial_context(Task &task);
    void set_current_task(int id);
    void exit();
    void force_finish(int id);
    void sleep(uint32_t ticks);
    int current_task();
    void yield();
    Task *get_task(int id);
    const char *state_string(TaskState state);
    void record_syscall();
    void record_memory_access(uint32_t count = 1);
    void set_protected(int id, bool value);
}
