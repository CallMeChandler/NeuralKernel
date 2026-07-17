#include "syscall.h"
#include "terminal.h"
#include "task.h"
#include "nn_scheduler.h"

namespace syscall
{
    void initialize()
    {
    }

    uint32_t handle(
        uint32_t number,
        uint32_t arg1,
        uint32_t arg2,
        uint32_t arg3)
    {
        nn_scheduler::record_syscall(task::current_task());
        (void)arg2;
        (void)arg3;

        switch (number)
        {
        case 0:
            terminal::write((const char *)arg1);
            return 0;

        case 1:
            task::sleep(arg1);
            return 0;

        case 2:
            task::exit();
            return 0;

        default:
            return (uint32_t)-1;
        }
    }
}

extern "C" uint32_t syscall_dispatch(
    uint32_t number,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3)
{
    return syscall::handle(number, arg1, arg2, arg3);
}
