#include "paging.h"
#include <stdint.h>
#include "terminal.h"

namespace paging
{
    //1024 entries per page directory/table
    //4KB pages

    uint32_t page_directory[1024] __attribute__((aligned(4096)));

    uint32_t first_page_table[1024] __attribute__((aligned(4096)));

    void initialize()
    {
        //clear page directory

        for (int i = 0; i < 1024; i++){
            page_directory[i] = 0x00000002;
        }

        //identity map first 4MB of memory

        for (int i = 0; i < 1024; i++){
            first_page_table[i] = (i * 0x1000) | 3;
        }

        //first entry of page directory points to first page table
        page_directory[0] = ((uint32_t)first_page_table) | 3;

        asm volatile(
            "mov %0, %%cr3"
            :
            : "r"(page_directory)
        );

        //enable paging
        uint32_t cr0;

        asm volatile(
            "mov %%cr0, %0"
            : "=r"(cr0)
        );



        cr0 |= 0x80000000;



        asm volatile(
            "mov %0, %%cr0"
            :
            : "r"(cr0)
        );

        terminal::write(
            "Paging enabled\n");
    }
}