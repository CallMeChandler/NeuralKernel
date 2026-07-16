#include "task.h"
#include "heap.h"
#include "scheduler.h"

namespace task
{
    constexpr int MAX_TASKS = 16;
    constexpr uint32_t TASK_STACK_SIZE = 4096;

    static Task tasks[MAX_TASKS];
    static int task_count = 0;
    static int current_running_task = 0;

    extern "C" void enter_user_mode(
        uint32_t entry,
        uint32_t user_stack_top);

    static void user_task_bootstrap()
    {
        Task *current = get_task(current_task());

        if (current == nullptr || !current->user_mode)
        {
            exit();
        }

        enter_user_mode(
            current->user_entry,
            current->user_stack_top);

        exit();
    }

    const char *state_string(TaskState state)
    {
        switch (state)
        {
        case TaskState::READY:
            return "READY";
        case TaskState::RUNNING:
            return "RUNNING";
        case TaskState::SLEEPING:
            return "SLEEP";
        case TaskState::FINISHED:
            return "DONE";
        }

        return "?";
    }

    void initialize()
    {
        task_count = 0;
        current_running_task = 0;

        for (int i = 0; i < MAX_TASKS; i++)
        {
            tasks[i].active = false;
        }
    }

    static int create_internal(
        const char *name,
        TaskFunction function,
        bool user_mode,
        uint32_t user_entry,
        uint32_t user_stack_top)
    {
        int slot = -1;
        for (int i = 0; i < MAX_TASKS; i++) if (!tasks[i].active) { slot = i; break; }
        if (slot < 0) return -1;

        Task &new_task = tasks[slot];
        new_task.id = slot;
        new_task.active = true;
        new_task.name = name;
        new_task.function = function;
        new_task.stack =
            (uint32_t *)heap::kmalloc(TASK_STACK_SIZE);

        if (new_task.stack == nullptr)
        {
            new_task.active = false;
            return -1;
        }

        new_task.kernel_stack_top =
            (uint32_t)((uint8_t *)new_task.stack + TASK_STACK_SIZE);
        new_task.esp = new_task.kernel_stack_top;
        new_task.state = TaskState::READY;
        new_task.wakeup_tick = 0;
        new_task.user_mode = user_mode;
        new_task.user_entry = user_entry;
        new_task.user_stack_top = user_stack_top;

        setup_initial_context(new_task);
        if (slot >= task_count) task_count = slot + 1;

        return new_task.id;
    }

    int create(const char *name, TaskFunction function)
    {
        return create_internal(name, function, false, 0, 0);
    }

    int create_user(
        const char *name,
        uint32_t entry,
        uint32_t user_stack_top)
    {
        if (entry == 0 || user_stack_top == 0)
        {
            return -1;
        }

        return create_internal(
            name,
            user_task_bootstrap,
            true,
            entry,
            user_stack_top);
    }

    Task *get_tasks()
    {
        return tasks;
    }

    int get_task_count()
    {
        return task_count;
    }

    void setup_initial_context(Task &target)
    {
        uint32_t *stack_top = (uint32_t *)target.esp;

        *(--stack_top) = (uint32_t)target.function;
        *(--stack_top) = 0; // EBP
        *(--stack_top) = 0; // EBX
        *(--stack_top) = 0; // ESI
        *(--stack_top) = 0; // EDI

        target.esp = (uint32_t)stack_top;
    }

    void set_current_task(int id)
    {
        current_running_task = id;
    }

    void exit()
    {
        tasks[current_running_task].state = TaskState::FINISHED;
        scheduler::schedule();

        while (true)
        {
            asm volatile("cli; hlt");
        }
    }

    int current_task()
    {
        return current_running_task;
    }

    void sleep(uint32_t ticks)
    {
        tasks[current_running_task].state = TaskState::SLEEPING;
        tasks[current_running_task].wakeup_tick =
            scheduler::get_ticks() + ticks;
        scheduler::schedule();
    }

    void yield()
    {
        scheduler::yield();
    }

    Task *get_task(int id)
    {
        if (id < 0 || id >= task_count)
        {
            return nullptr;
        }

        return &tasks[id];
    }
}
