#include "task.h"
#include "heap.h"

namespace task
{
    constexpr int MAX_TASKS = 16;

    constexpr uint32_t TASK_STACK_SIZE = 4096;

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

        Task &task = tasks[task_count];
        task.id = task_count;
        task.active = true;
        task.function = function;
        task.stack =
            (uint32_t *)heap::kmalloc(
                TASK_STACK_SIZE);

        if (task.stack == nullptr)
        {
            return -1;
        }

        task.esp =
            (uint32_t)((uint8_t *)task.stack +
                       TASK_STACK_SIZE);

        task.state = TaskState::READY;

        // setup_initial_context(task);
        
        task_count++;

        return task.id;
    }

    Task *get_tasks()
    {
        return tasks;
    }

    int get_task_count()
    {
        return task_count;
    }

    void setup_initial_context(Task &task){
        uint32_t *stack_top = (uint32_t *)task.esp;

        stack_top--;

        *stack_top = (uint32_t)task.function;

        task.esp = (uint32_t)stack_top;
    }
}