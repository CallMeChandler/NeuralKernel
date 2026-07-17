#pragma once
#include <stdint.h>
#include "task.h"

namespace nn_scheduler
{
    enum class TaskClass : uint8_t
    {
        INTERACTIVE,
        CPU_BOUND,
        IO_BOUND,
        BATCH
    };

    struct Metrics
    {
        uint32_t runtime_ticks;
        uint32_t ready_ticks;
        uint32_t sleep_ticks;
        uint32_t slices;
        uint32_t sleeps;
        uint32_t syscalls;
        uint32_t memory_accesses;
        uint32_t last_run_tick;
        uint32_t burst_ticks;
        uint32_t last_burst_ticks;
    };

    void initialize();
    void set_enabled(bool value);
    bool enabled();
    void register_task(int id);
    void record_sleep(int id);
    void record_syscall(int id);
    void record_memory_access(int id, uint32_t count = 1);
    void on_tick(task::Task *tasks, int count, int current_task, uint32_t now);
    int select_next(task::Task *tasks, int count, int old_task, uint32_t now);
    void set_penalty(int id, int8_t penalty);
    const Metrics *metrics_for(int id);
    TaskClass class_for(int id);
    int32_t score_for(int id);
    const char *class_name(TaskClass value);
    uint32_t decisions();
}
