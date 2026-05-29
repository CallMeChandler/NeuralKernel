#include "pmm.h"
#include "terminal.h"

namespace pmm
{
    constexpr uint32_t PAGE_SIZE = 4096;

    constexpr uint32_t TOTAL_PAGES = 4096;

    constexpr uint32_t MEMORY_START =
        0x01000000;

    static bool used_pages[TOTAL_PAGES];

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
        for (uint32_t i = 0; i < TOTAL_PAGES; i++)
        {
            used_pages[i] = false;
        }

        for (uint32_t i = 0; i < 256; i++)
        {
            used_pages[i] = true;
        }
    }

    void *alloc_page()
    {
        for (uint32_t i = 0; i < TOTAL_PAGES; i++)
        {
            if (!used_pages[i])
            {
                used_pages[i] = true;

                uint32_t address = MEMORY_START + (i * PAGE_SIZE);

                return (void *)address;
            }
        }

        return nullptr;
    }

    void free_page(void *page)
    {
        uint32_t address = (uint32_t)page;

        if (address < MEMORY_START)
        {
            return;
        }

        uint32_t index = (address - MEMORY_START) / PAGE_SIZE;
        if (index < TOTAL_PAGES)
        {
            used_pages[index] = false;
        }
    }

    void print_stats()
    {
        uint32_t used = 0;

        uint32_t free = 0;

        for (uint32_t i = 0;
             i < TOTAL_PAGES;
             i++)
        {
            if (used_pages[i])
            {
                used++;
            }
            else
            {
                free++;
            }
        }

        terminal::write(
            "PMM Stats:\n");

        terminal::write(
            "Used pages: ");

        print_number(used);

        terminal::putchar('\n');

        terminal::write(
            "Free pages: ");

        print_number(free);

        terminal::putchar('\n');
    }

}