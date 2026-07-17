#include "nn_scheduler.h"
#include "nn.h"
#include "nn_weights.h"

namespace nn_scheduler
{
    constexpr int MAX_TASKS = 16;
    static Metrics metrics[MAX_TASKS];
    static TaskClass classes[MAX_TASKS];
    static int32_t scores[MAX_TASKS];
    static int8_t penalties[MAX_TASKS];
    static bool active = false;
    static uint32_t decision_count = 0;

    static nn::q8_8 normalize(uint32_t value, uint32_t maximum)
    {
        if (!maximum) return 0;
        if (value >= maximum) return 256;
        return (nn::q8_8)((value * 256U) / maximum);
    }

    static nn::q8_8 inverse(uint32_t value, uint32_t maximum)
    {
        return (nn::q8_8)(256 - normalize(value, maximum));
    }

    static int32_t evaluate(int id, uint32_t now, uint32_t rr_position)
    {
        Metrics &m = metrics[id];
        uint32_t slices = m.slices ? m.slices : 1;
        uint32_t average_runtime = m.runtime_ticks / slices;
        uint32_t syscall_rate = m.syscalls / slices;
        uint32_t sleep_frequency = m.sleeps / slices;
        uint32_t total_observed = m.runtime_ticks + m.sleep_ticks + 1;
        uint32_t io_ratio = m.sleep_ticks / total_observed;
        uint32_t since_last_run = now - m.last_run_tick;

        nn::q8_8 input[8] = {
            normalize(average_runtime, 20),
            normalize(sleep_frequency, 4),
            normalize(syscall_rate, 12),
            normalize(io_ratio, 1),
            normalize(m.last_burst_ticks, 20),
            normalize(m.ready_ticks, 100),
            normalize(since_last_run, 100),
            normalize(m.memory_accesses / slices, 64)
        };

        nn::q8_8 hidden[12];
        nn::q8_8 class_scores[4];
        nn::q8_8 policy_weights[5];

        const nn::Layer hidden_layer = {
            nn_weights::scheduler_hidden_weights,
            nn_weights::scheduler_hidden_bias,
            8, 12, nn::Activation::RELU
        };
        const nn::Layer class_layer = {
            nn_weights::scheduler_class_weights,
            nn_weights::scheduler_class_bias,
            12, 4, nn::Activation::NONE
        };
        const nn::Layer policy_layer = {
            nn_weights::scheduler_policy_weights,
            nn_weights::scheduler_policy_bias,
            4, 5, nn::Activation::RELU
        };

        nn::forward(hidden_layer, input, hidden);
        nn::forward(class_layer, hidden, class_scores);
        nn::forward(policy_layer, class_scores, policy_weights);

        const nn::q8_8 classical[5] = {
            normalize(since_last_run, 100),
            inverse(average_runtime, 20),
            inverse(rr_position, 16),
            128,
            256
        };

        int32_t composite = penalties[id] * 16;
        for (int i = 0; i < 5; i++)
            composite += nn::mul(policy_weights[i], classical[i]);

        classes[id] = (TaskClass)nn::argmax(class_scores, 4);
        scores[id] = composite;
        decision_count++;
        return composite;
    }

    void initialize()
    {
        active = false;
        decision_count = 0;
        for (int i = 0; i < MAX_TASKS; i++)
        {
            metrics[i] = {};
            classes[i] = TaskClass::BATCH;
            scores[i] = 0;
            penalties[i] = 0;
        }
    }

    void set_enabled(bool value) { active = value; }
    bool enabled() { return active; }

    void register_task(int id)
    {
        if (id < 0 || id >= MAX_TASKS) return;
        metrics[id] = {};
        classes[id] = TaskClass::BATCH;
        scores[id] = 0;
        penalties[id] = 0;
    }

    void record_sleep(int id)
    {
        if (id < 0 || id >= MAX_TASKS) return;
        metrics[id].sleeps++;
        metrics[id].last_burst_ticks = metrics[id].burst_ticks;
        metrics[id].burst_ticks = 0;
    }

    void record_syscall(int id)
    {
        if (id >= 0 && id < MAX_TASKS) metrics[id].syscalls++;
    }

    void record_memory_access(int id, uint32_t count)
    {
        if (id >= 0 && id < MAX_TASKS) metrics[id].memory_accesses += count;
    }

    void on_tick(task::Task *tasks, int count, int current_task, uint32_t now)
    {
        (void)now;
        if (!tasks) return;
        if (count > MAX_TASKS) count = MAX_TASKS;

        for (int i = 0; i < count; i++)
        {
            if (!tasks[i].active) continue;
            if (tasks[i].state == task::TaskState::RUNNING && i == current_task)
            {
                metrics[i].runtime_ticks++;
                metrics[i].burst_ticks++;
            }
            else if (tasks[i].state == task::TaskState::READY)
            {
                metrics[i].ready_ticks++;
            }
            else if (tasks[i].state == task::TaskState::SLEEPING)
            {
                metrics[i].sleep_ticks++;
            }
        }
    }

    int select_next(task::Task *tasks, int count, int old_task, uint32_t now)
    {
        if (!active || !tasks || count <= 1) return -1;
        if (count > MAX_TASKS) count = MAX_TASKS;

        int selected = -1;
        int32_t best_score = -2147483647;
        uint32_t rr_position = 0;

        for (int offset = 1; offset <= count; offset++)
        {
            int id = (old_task + offset) % count;
            if (id == 0 || !tasks[id].active || tasks[id].state != task::TaskState::READY)
                continue;

            int32_t value = evaluate(id, now, rr_position++);
            if (selected < 0 || value > best_score)
            {
                selected = id;
                best_score = value;
            }
        }

        return selected;
    }

    void set_penalty(int id, int8_t penalty)
    {
        if (id >= 0 && id < MAX_TASKS) penalties[id] = penalty;
    }

    const Metrics *metrics_for(int id)
    {
        return id >= 0 && id < MAX_TASKS ? &metrics[id] : nullptr;
    }

    TaskClass class_for(int id)
    {
        return id >= 0 && id < MAX_TASKS ? classes[id] : TaskClass::BATCH;
    }

    int32_t score_for(int id)
    {
        return id >= 0 && id < MAX_TASKS ? scores[id] : 0;
    }

    const char *class_name(TaskClass value)
    {
        switch (value)
        {
        case TaskClass::INTERACTIVE: return "INTERACTIVE";
        case TaskClass::CPU_BOUND: return "CPU_BOUND";
        case TaskClass::IO_BOUND: return "IO_BOUND";
        case TaskClass::BATCH: return "BATCH";
        }
        return "UNKNOWN";
    }

    uint32_t decisions() { return decision_count; }
}
