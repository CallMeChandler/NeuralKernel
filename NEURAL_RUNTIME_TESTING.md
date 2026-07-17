# NeuralKernel Neural Runtime — Build and Test Guide

This patch adds the first three neural subsystems while preserving the existing
keyboard, authentication, NKFS, editor, ELF, and Ring 3 paths.

## Build

No new host dependency is required.

```bash
make clean
make run
```

The checked-in `kernel/nn_weights.h` is already usable. To verify/regenerate the
offline export artifact before building:

```bash
python3 tools/train_all.py
make clean
make run
```

## Expected boot

Boot, login, shell input, NKFS commands, editor input, and the Ring 3 ELF should
behave exactly as before. The old `worker1`, `worker2`, and `sysdemo` startup
tasks are no longer created, so `12SYS` must not appear in the shell.

Default credentials remain whatever is stored in `nkfs_files/users`.

## Neural scheduler

Run:

```text
ps
```

The table now includes a neural behavioral class and watchdog score:

```text
ID STATE CLASS SCORE NAME
```

Run:

```text
neuralstats
```

`Scheduler decisions` should increase as tasks are selected. The scheduler uses
Q8.8 inference to blend FCFS, estimated SJF, RR, dynamic priority, and MLFQ
scores. Round-robin order remains the deterministic tiebreaking order.

The shell itself remains IRQ-driven in this architecture; it is intentionally
not converted into a scheduler task because that would risk the working
keyboard path. Neural classification applies to actual scheduler tasks.

## Neural PMM predictor

Normal pages allocated with `pmm::alloc_page()` are never evicted. This protects
kernel structures, ELF pages, and user stacks.

The neural eviction path is active only for pages allocated through:

```cpp
void *page = pmm::alloc_reclaimable_page(task_id, size_class);
pmm::touch_page(page, true);
```

When physical memory is exhausted, the PMM scores only explicitly reclaimable
pages and recycles the lowest-utility candidate. This is necessary because the
current kernel has no swap device or general page-owner unmapping.

Check counters with:

```text
pmm
neuralstats
```

## Anomaly watchdog

A protected `watchdog` kernel task samples every 50 scheduler ticks. It observes
runtime share, syscall deltas, sleep-pattern changes, memory growth, free-page
trend, starvation, page faults, and IRQ rate.

Run:

```text
ps
neuralstats
```

For normal workloads, warnings and interventions should remain zero. Repeated
high reconstruction error first logs a warning, then lowers task scheduling
bias, and only terminates an unprotected task after four severe samples.
`idle` and `watchdog` are protected.

## Regression checklist

1. Boot splash and colored logs render.
2. Login accepts the configured `/users` credentials.
3. Keyboard input and backspace work.
4. `ls`, `cat`, `write`, `rm`, and `edit` still work.
5. Ctrl+S and Ctrl+Q work in the editor.
6. `hello.elf` runs in Ring 3 and exits normally.
7. `ps` shows `idle`, `watchdog`, and any active user task.
8. `neuralstats` shows increasing scheduler decisions.
9. No `1`, `2`, or `SYS` text is printed in the background.
10. No watchdog warning appears during ordinary shell use.

## Files intentionally untouched

The keyboard and low-level IRQ input files are not changed by this patch:

- `kernel/keyboard.cpp`
- `kernel/keyboard.asm`
- `kernel/irq.cpp`
- `kernel/irq.asm`

## Important PMM limitation

The predictor is real and integrated, but safe replacement requires the caller
to mark disposable pages as reclaimable. Arbitrary live-page eviction would be
incorrect until NeuralKernel has per-process page tables, unmapping, and swap or
reload callbacks.
