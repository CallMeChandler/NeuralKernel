#include "scheduler.h"
#include "task.h"
#include "context.h"
#include "gdt.h"

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

    void schedule()
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
            task::set_current_task(current_task);
            tasks[current_task].state = task::TaskState::RUNNING;
            gdt::set_kernel_stack(tasks[current_task].kernel_stack_top);

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
                    task::TaskState::READY &&
                next_task != 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            next_task = 0;
        }

        if (tasks[old_task].state == task::TaskState::RUNNING)
        {
            tasks[old_task].state = task::TaskState::READY;
        }

        current_task = next_task;
        tasks[current_task].state = task::TaskState::RUNNING;

        task::set_current_task(current_task);
        gdt::set_kernel_stack(tasks[current_task].kernel_stack_top);

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

    void yield()
    {
        schedule();
    }
}