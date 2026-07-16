#include "heap.h"
#include "terminal.h"
#include <stdint.h>

namespace heap
{
    constexpr uint32_t HEAP_START = 0x00200000;
    constexpr uint32_t HEAP_SIZE = 1024 * 1024;

    struct Block
    {
        uint32_t size;
        bool free;
        Block* next;
    };

    static Block* head = nullptr;
    static uint32_t used_bytes = 0;

    static void print_number(uint32_t n)
    {
        if (n == 0) { terminal::putchar('0'); return; }
        char buffer[16]; int i = 0;
        while (n) { buffer[i++] = '0' + (n % 10); n /= 10; }
        while (i--) terminal::putchar(buffer[i]);
    }

    void initialize()
    {
        head = (Block*)HEAP_START;
        head->size = HEAP_SIZE - sizeof(Block);
        head->free = true;
        head->next = nullptr;
        used_bytes = 0;
    }

    void* kmalloc(size_t requested)
    {
        if (!requested || !head) return nullptr;
        uint32_t size = ((uint32_t)requested + 7) & ~7U;

        for (Block* block = head; block; block = block->next)
        {
            if (!block->free || block->size < size) continue;

            if (block->size >= size + sizeof(Block) + 8)
            {
                Block* split = (Block*)((uint8_t*)(block + 1) + size);
                split->size = block->size - size - sizeof(Block);
                split->free = true;
                split->next = block->next;
                block->next = split;
                block->size = size;
            }

            block->free = false;
            used_bytes += block->size;
            return block + 1;
        }
        return nullptr;
    }

    void kfree(void* pointer)
    {
        if (!pointer) return;
        Block* block = ((Block*)pointer) - 1;
        if (block->free) return;

        block->free = true;
        if (used_bytes >= block->size) used_bytes -= block->size;

        for (Block* current = head; current && current->next; )
        {
            if (current->free && current->next->free)
            {
                current->size += sizeof(Block) + current->next->size;
                current->next = current->next->next;
            }
            else current = current->next;
        }
    }

    uint32_t allocated_bytes() { return used_bytes; }

    uint32_t free_bytes()
    {
        uint32_t total = 0;
        for (Block* block = head; block; block = block->next)
            if (block->free) total += block->size;
        return total;
    }

    void print_stats()
    {
        terminal::write("Heap Stats:\nAllocated bytes: ");
        print_number(allocated_bytes());
        terminal::write("\nFree bytes: ");
        print_number(free_bytes());
        terminal::putchar('\n');
    }
}
