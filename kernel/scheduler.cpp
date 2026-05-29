#include "scheduler.h"
#include "task.h"

namespace scheduler
{
    static int current_task = 0;

    void initialize()
    {
        current_task = 0;
    }

    void run()
    {
        task::Task* tasks =
            task::get_tasks();

        int count =
            task::get_task_count();

        if (count == 0)
        {
            return;
        }

        if (current_task >= count)
        {
            current_task = 0;
        }

        if (tasks[current_task].active)
        {
            tasks[current_task].function();
        }

        current_task++;
    }
}