#include "scheduler.h"
#include "task.h"
#include "context.h"

namespace scheduler
{
    static int current_task = 0;
    static bool started = false;
    static volatile int schedule_request = 0;

    void initialize()
    {
        current_task = 0;
        started = false;
    }

    void run()
    {
        schedule_request = 0;
        task::Task* tasks =
            task::get_tasks();

        int count =
            task::get_task_count();

        if(count == 0)
        {
            return;
        }

        if(!started)
        {
            started = true;

            current_task = 0;

            context_switch(
                nullptr,
                tasks[0].esp
            );

            return;
        }

        int old_task =
            current_task;

        current_task =
            (current_task + 1) % count;

        context_switch(
            &tasks[old_task].esp,
            tasks[current_task].esp
        );
    }

    void tick()
    {
        schedule_request = 1;
    }

    bool should_schedule()
    {
        return schedule_request;
    }
}