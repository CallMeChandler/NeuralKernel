#include "task.h"

namespace task
{
    constexpr int MAX_TASKS = 16;

    static Task tasks[MAX_TASKS];

    static int task_count = 0;

    void initialize()
    {
        task_count = 0;

        for (int i = 0; i < MAX_TASKS; i++)
        {
            tasks[i].active = false;
        }
    }

    int create(TaskFunction function)
    {
        if (task_count >= MAX_TASKS)
        {
            return -1;
        }

        Task& task = tasks[task_count];
        task.id = task_count;
        task.active = true;
        task.function = function;
        task_count++;

        return task.id;
    }

    Task* get_tasks()
    {
        return tasks;
    }

    int get_task_count()
    {
        return task_count;
    }
}