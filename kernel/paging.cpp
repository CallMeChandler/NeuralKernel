#include "paging.h"
#include <stdint.h>
#include "terminal.h"

namespace paging
{
    // 1024 entries per page directory/table
    // 4KB pages

    uint32_t page_directory[1024] __attribute__((aligned(4096)));

    constexpr int PAGE_TABLE_COUNT = 17;

    uint32_t page_tables
        [PAGE_TABLE_COUNT]
        [1024]
        __attribute__((aligned(4096)));

    void initialize()
    {
        // clear page directory

        for (int i = 0; i < 1024; i++)
        {
            page_directory[i] = 0x00000002;
        }

        // identity map first 4MB of memory

        for (int table = 0;
             table < PAGE_TABLE_COUNT;
             table++)
        {
            for (int page = 0;
                 page < 1024;
                 page++)
            {
                uint32_t address =
                    (table * 1024 + page) * 0x1000;

                page_tables[table][page] =
                    address | 3;
            }
        }

        // first entry of page directory points to first page table
        for (int table = 0;
             table < PAGE_TABLE_COUNT;
             table++)
        {
            page_directory[table] =
                ((uint32_t)page_tables[table]) | 3;
        }

        asm volatile(
            "mov %0, %%cr3"
            :
            : "r"(page_directory));

        // enable paging
        uint32_t cr0;

        asm volatile(
            "mov %%cr0, %0"
            : "=r"(cr0));

        cr0 |= 0x80000000;

        asm volatile(
            "mov %0, %%cr0"
            :
            : "r"(cr0));

        terminal::write(
            "Paging enabled\n");
    }
}