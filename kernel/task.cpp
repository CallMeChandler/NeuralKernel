#include "task.h"
#include "heap.h"
#include "scheduler.h"

namespace task
{
    constexpr int MAX_TASKS = 16;

    constexpr uint32_t TASK_STACK_SIZE = 4096;

    static Task tasks[MAX_TASKS];

    static int task_count = 0;

    static int current_running_task = -0;

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

        setup_initial_context(task);

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

    void setup_initial_context(Task &task)
    {
        uint32_t *stack_top =
            (uint32_t *)task.esp;

        *(--stack_top) =
            (uint32_t)task.function; // RET target

        *(--stack_top) = 0; // EBP
        *(--stack_top) = 0; // EBX
        *(--stack_top) = 0; // ESI
        *(--stack_top) = 0; // EDI

        task.esp =
            (uint32_t)stack_top;
    }

    void set_current_task(int id)
    {
        current_running_task = id;
    }

    void exit()
    {
        tasks[current_running_task].state =
            TaskState::FINISHED;

        scheduler::run();

        while (true)
        {
        }
    }

    int current_task()
    {
        return current_running_task;
    }

    void sleep(uint32_t ticks)
    {
        tasks[current_running_task].state =
            TaskState::SLEEPING;

        tasks[current_running_task].wakeup_tick =
            scheduler::get_ticks() + ticks;

        scheduler::run();
    }
}