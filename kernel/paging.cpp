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

    bool make_user_accessible(uint32_t start, uint32_t size)
    {
        if (size == 0)
        {
            return true;
        }

        uint32_t first_page = start & 0xFFFFF000;
        uint32_t last_address = start + size - 1;

        if (last_address < start)
        {
            return false;
        }

        uint32_t last_page = last_address & 0xFFFFF000;

        for (uint32_t address = first_page;; address += 0x1000)
        {
            uint32_t directory_index = address >> 22;
            uint32_t table_index = (address >> 12) & 0x3FF;

            if (directory_index >= PAGE_TABLE_COUNT)
            {
                return false;
            }

            page_directory[directory_index] |= 0x4;
            page_tables[directory_index][table_index] |= 0x4;

            asm volatile("invlpg (%0)" : : "r"((void *)address) : "memory");

            if (address == last_page)
            {
                break;
            }
        }

        return true;
    }

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