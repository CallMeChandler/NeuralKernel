#include "heap.h"

#include "terminal.h"

#include <stdint.h>

namespace heap
{
    constexpr uint32_t HEAP_START =
        0x00200000; // 64 MiB

    constexpr uint32_t HEAP_SIZE = 1024 * 1024; // 1 MiB

    static uint32_t heap_current = HEAP_START;

    static uint32_t heap_end = HEAP_START + HEAP_SIZE;

    static uint32_t total_allocated = 0;

    static void print_number(uint32_t n)
    {
        if (n == 0)
        {
            terminal::putchar('0');
            return;
        }

        char buffer[16];

        int i = 0;

        while (n > 0)
        {
            buffer[i] =
                '0' + (n % 10);

            n /= 10;

            i++;
        }

        for (int j = i - 1; j >= 0; j--)
        {
            terminal::putchar(
                buffer[j]);
        }
    }

    void initialize()
    {
        heap_current = HEAP_START;

        total_allocated = 0;
    }

    void *kmalloc(size_t size)
    {
        // allign to 8 bytes

        size = (size + 7) & ~7;

        if (heap_current + size >= heap_end)
        {
            return nullptr; // Out of memory
        }

        void *address = (void *)heap_current;

        heap_current += size;
        total_allocated += size;

        return address;
    }

    void print_stats()
    {
        terminal::write(
            "Heap Stats:\n");

        terminal::write(
            "Allocated bytes: ");

        print_number(
            total_allocated);

        terminal::putchar('\n');

        terminal::write(
            "Remaining bytes: ");

        print_number(
            heap_end - heap_current);

        terminal::putchar('\n');
    }
}