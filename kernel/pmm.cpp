#include "pmm.h"
#include "terminal.h"
#include "scheduler.h"
#include "nn_pmm.h"
#include "nn_scheduler.h"

namespace pmm
{
    constexpr uint32_t PAGE_SIZE = 4096;
    constexpr uint32_t TOTAL_PAGES = 4096;
    constexpr uint32_t MEMORY_START = 0x01000000;

    struct PageMeta
    {
        uint32_t allocated_tick;
        uint32_t last_access_tick;
        uint16_t access_count;
        uint16_t owner_task;
        uint16_t stride;
        uint8_t size_class;
        uint8_t flags;
    };

    constexpr uint8_t RECLAIMABLE = 1;
    constexpr uint8_t DIRTY = 2;

    static bool used_pages[TOTAL_PAGES];
    static PageMeta metadata[TOTAL_PAGES];
    static uint32_t eviction_count = 0;
    static int32_t eviction_score = 0;

    static void print_number(uint32_t n)
    {
        if (!n) { terminal::putchar('0'); return; }
        char buffer[16]; int i = 0;
        while (n) { buffer[i++] = '0' + n % 10; n /= 10; }
        while (i--) terminal::putchar(buffer[i]);
    }

    static int page_index(void *page)
    {
        uint32_t address = (uint32_t)page;
        if (address < MEMORY_START) return -1;
        uint32_t index = (address - MEMORY_START) / PAGE_SIZE;
        return index < TOTAL_PAGES ? (int)index : -1;
    }

    static void prepare_page(uint32_t index, uint32_t owner, bool reclaimable, uint8_t size_class)
    {
        used_pages[index] = true;
        PageMeta &meta = metadata[index];
        meta.allocated_tick = scheduler::get_ticks();
        meta.last_access_tick = meta.allocated_tick;
        meta.access_count = 1;
        meta.owner_task = (uint16_t)owner;
        meta.stride = 0;
        meta.size_class = size_class > 3 ? 3 : size_class;
        meta.flags = reclaimable ? RECLAIMABLE : 0;
    }

    static int select_victim()
    {
        int selected = -1;
        int32_t best_score = 2147483647;
        uint32_t now = scheduler::get_ticks();

        for (uint32_t i = 256; i < TOTAL_PAGES; i++)
        {
            if (!used_pages[i] || !(metadata[i].flags & RECLAIMABLE)) continue;
            const PageMeta &meta = metadata[i];
            nn_pmm::Features features = {
                now - meta.last_access_tick,
                meta.access_count,
                now - meta.allocated_tick,
                meta.owner_task,
                meta.stride,
                (meta.flags & DIRTY) != 0,
                meta.size_class,
                0
            };
            int32_t value = nn_pmm::score(features);
            if (selected < 0 || value < best_score)
            {
                selected = (int)i;
                best_score = value;
            }
        }

        eviction_score = best_score;
        return selected;
    }

    void initialize()
    {
        for (uint32_t i = 0; i < TOTAL_PAGES; i++)
        {
            used_pages[i] = false;
            metadata[i] = {};
        }
        for (uint32_t i = 0; i < 256; i++) used_pages[i] = true;
        eviction_count = 0;
        eviction_score = 0;
    }

    void *alloc_page()
    {
        // Keep the known-working normal allocation path entirely classical.
        for (uint32_t i = 0; i < TOTAL_PAGES; i++)
        {
            if (!used_pages[i])
            {
                prepare_page(i, 0, false, 0);
                return (void *)(MEMORY_START + i * PAGE_SIZE);
            }
        }
        return nullptr;
    }

    void *alloc_reclaimable_page(uint32_t owner_task, uint8_t size_class)
    {
        for (uint32_t i = 256; i < TOTAL_PAGES; i++)
        {
            if (!used_pages[i])
            {
                prepare_page(i, owner_task, true, size_class);
                nn_scheduler::record_memory_access((int)owner_task);
                return (void *)(MEMORY_START + i * PAGE_SIZE);
            }
        }

        int victim = select_victim();
        if (victim < 0) return nullptr;
        prepare_page((uint32_t)victim, owner_task, true, size_class);
        eviction_count++;
        nn_scheduler::record_memory_access((int)owner_task);
        return (void *)(MEMORY_START + (uint32_t)victim * PAGE_SIZE);
    }

    void free_page(void *page)
    {
        int index = page_index(page);
        if (index < 256) return;
        used_pages[index] = false;
        metadata[index] = {};
    }

    void touch_page(void *page, bool written)
    {
        int index = page_index(page);
        if (index < 0 || !used_pages[index]) return;
        PageMeta &meta = metadata[index];
        meta.last_access_tick = scheduler::get_ticks();
        if (meta.access_count != 0xFFFF) meta.access_count++;
        if (written) meta.flags |= DIRTY;
        nn_scheduler::record_memory_access(meta.owner_task);
    }

    bool set_reclaimable(void *page, bool reclaimable)
    {
        int index = page_index(page);
        if (index < 256 || !used_pages[index]) return false;
        if (reclaimable) metadata[index].flags |= RECLAIMABLE;
        else metadata[index].flags &= (uint8_t)~RECLAIMABLE;
        return true;
    }

    uint32_t used_pages_count()
    {
        uint32_t used = 0;
        for (uint32_t i = 0; i < TOTAL_PAGES; i++) if (used_pages[i]) used++;
        return used;
    }

    uint32_t free_pages_count() { return TOTAL_PAGES - used_pages_count(); }
    uint32_t neural_evictions() { return eviction_count; }
    int32_t last_eviction_score() { return eviction_score; }

    void print_stats()
    {
        terminal::write("PMM Stats:\nUsed pages: ");
        print_number(used_pages_count());
        terminal::write("\nFree pages: ");
        print_number(free_pages_count());
        terminal::write("\nNeural evictions: ");
        print_number(eviction_count);
        terminal::putchar('\n');
    }
}
