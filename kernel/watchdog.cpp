#include "watchdog.h"
#include "nn.h"
#include "nn_weights.h"
#include "nn_scheduler.h"
#include "task.h"
#include "scheduler.h"
#include "telemetry.h"
#include "pmm.h"
#include "heap.h"
#include "printk.h"

namespace watchdog
{
    constexpr int MAX_TASKS = 16;
    static bool active = false;
    static uint32_t previous_free_pages = 0;
    static uint32_t previous_heap = 0;
    static uint32_t previous_irqs = 0;
    static uint32_t last_sample_tick = 0;
    static uint32_t score_value = 0;
    static uint32_t warning_count = 0;
    static uint32_t intervention_count = 0;
    static uint32_t runtime_snapshot[MAX_TASKS];
    static uint32_t syscall_snapshot[MAX_TASKS];
    static uint8_t strikes[MAX_TASKS];

    static nn::q8_8 normalize(uint32_t value, uint32_t maximum)
    {
        if (!maximum) return 0;
        if (value >= maximum) return 256;
        return (nn::q8_8)((value * 256U) / maximum);
    }

    static uint32_t evaluate(int id, const nn_scheduler::Metrics &metrics)
    {
        uint32_t runtime_delta = metrics.runtime_ticks - runtime_snapshot[id];
        uint32_t syscall_delta = metrics.syscalls - syscall_snapshot[id];
        uint32_t free_pages = pmm::free_pages_count();
        uint32_t heap_bytes = heap::allocated_bytes();
        uint32_t irq_total = telemetry::timer_ticks + telemetry::keyboard_irqs;
        uint32_t memory_growth = heap_bytes > previous_heap ? heap_bytes - previous_heap : 0;
        uint32_t free_page_drop = previous_free_pages > free_pages ? previous_free_pages - free_pages : 0;
        uint32_t irq_delta = irq_total - previous_irqs;

        nn::q8_8 input[8] = {
            normalize(syscall_delta, 64),
            normalize(memory_growth, 64 * 1024),
            normalize(runtime_delta, 50),
            normalize(metrics.sleeps == 0 ? runtime_delta : 0, 50),
            0,
            normalize(free_page_drop, 32),
            normalize(metrics.ready_ticks, 100),
            normalize(irq_delta > 1000 ? irq_delta - 1000 : 0, 1000)
        };

        nn::q8_8 encoded[4];
        nn::q8_8 reconstructed[8];
        const nn::Layer encoder = {
            nn_weights::watchdog_encoder_weights,
            nn_weights::watchdog_encoder_bias,
            8, 4, nn::Activation::RELU
        };
        const nn::Layer decoder = {
            nn_weights::watchdog_decoder_weights,
            nn_weights::watchdog_decoder_bias,
            4, 8, nn::Activation::RELU
        };
        nn::forward(encoder, input, encoded);
        nn::forward(decoder, encoded, reconstructed);

        runtime_snapshot[id] = metrics.runtime_ticks;
        syscall_snapshot[id] = metrics.syscalls;
        return nn::mse(input, reconstructed, 8);
    }

    void initialize()
    {
        active = false;
        previous_free_pages = pmm::free_pages_count();
        previous_heap = heap::allocated_bytes();
        previous_irqs = telemetry::timer_ticks + telemetry::keyboard_irqs;
        last_sample_tick = 0;
        score_value = warning_count = intervention_count = 0;
        for (int i = 0; i < MAX_TASKS; i++)
        {
            runtime_snapshot[i] = 0;
            syscall_snapshot[i] = 0;
            strikes[i] = 0;
        }
    }

    void set_enabled(bool value)
    {
        active = value;
        last_sample_tick = scheduler::get_ticks();
    }

    bool enabled() { return active; }

    void poll()
    {
        if (!active) return;
        uint32_t now = scheduler::get_ticks();
        if (now - last_sample_tick < 50) return;
        last_sample_tick = now;

        task::Task *tasks = task::get_tasks();
        int count = task::get_task_count();
        if (count > MAX_TASKS) count = MAX_TASKS;
        uint32_t highest = 0;

        for (int i = 1; i < count; i++)
        {
            if (!tasks[i].active || tasks[i].state == task::TaskState::FINISHED) continue;
            if (i == task::current_task()) continue;
            const nn_scheduler::Metrics *metrics = nn_scheduler::metrics_for(i);
            if (!metrics) continue;

            uint32_t error = evaluate(i, *metrics);
            if (error > highest) highest = error;

            if (error > 23000)
            {
                if (strikes[i] < 255) strikes[i]++;
                if (strikes[i] == 1)
                {
                    printk::log(printk::WARN, "Watchdog: elevated task anomaly");
                    warning_count++;
                }
                else if (strikes[i] == 2)
                {
                    nn_scheduler::set_penalty(i, -4);
                    printk::log(printk::WARN, "Watchdog: task scheduling reduced");
                    intervention_count++;
                }
                else if (strikes[i] >= 4)
                {
                    tasks[i].state = task::TaskState::FINISHED;
                    printk::log(printk::ERROR, "Watchdog: runaway task terminated");
                    intervention_count++;
                }
            }
            else if (strikes[i])
            {
                strikes[i]--;
            }
        }

        score_value = highest;
        previous_free_pages = pmm::free_pages_count();
        previous_heap = heap::allocated_bytes();
        previous_irqs = telemetry::timer_ticks + telemetry::keyboard_irqs;
    }

    uint32_t last_score() { return score_value; }
    uint32_t warnings() { return warning_count; }
    uint32_t interventions() { return intervention_count; }
}
