#include "scheduler.h"
#include "task.h"
#include "context.h"

namespace scheduler
{
    static int current_task = 0;
    static bool started = false;
    static volatile int schedule_request = 0;
    static uint32_t kernel_ticks = 0;

    void initialize()
    {
        current_task = 0;
        started = false;
    }

    void run()
    {
        schedule_request = 0;

        task::Task *tasks =
            task::get_tasks();

        int count =
            task::get_task_count();

        if (count == 0)
        {
            return;
        }

        if (!started)
        {
            started = true;

            current_task = 0;

            context_switch(
                nullptr,
                tasks[0].esp);

            return;
        }

        int old_task =
            current_task;

        int next_task =
            current_task;

        bool found = false;

        for (int i = 0; i < count; i++)
        {
            next_task =
                (next_task + 1) % count;

            if (tasks[next_task].state ==
                task::TaskState::READY)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return;
        }

        current_task =
            next_task;

        task::set_current_task(
            current_task);

        context_switch(
            &tasks[old_task].esp,
            tasks[current_task].esp);
    }

    void tick()
    {
        kernel_ticks++;

        schedule_request = 1;

        task::Task *tasks =
            task::get_tasks();

        int count =
            task::get_task_count();

        for (int i = 0; i < count; i++)
        {
            if (tasks[i].state ==
                task::TaskState::SLEEPING)
            {
                if (kernel_ticks >=
                    tasks[i].wakeup_tick)
                {
                    tasks[i].state =
                        task::TaskState::READY;
                }
            }
        }
    }

    bool should_schedule()
    {
        return schedule_request;
    }

    uint32_t get_ticks()
    {
        return kernel_ticks;
    }
}