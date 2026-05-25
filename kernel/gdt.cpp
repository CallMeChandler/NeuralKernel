#include "gdt.h"
#include "printk.h"

namespace gdt{
    struct Entry{
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } __attribute__((packed));

    struct Pointer{
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    static Entry gdt_entries[3];
    static Pointer gdt_ptr;

    static void set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
    {
        gdt_entries[i].base_low = base & 0xFFFF;
        gdt_entries[i].base_middle = (base >> 16) & 0xFF;
        gdt_entries[i].base_high = (base >> 24) & 0xFF;

        gdt_entries[i].limit_low = limit & 0xFFFF;
        gdt_entries[i].granularity = (limit >> 16) & 0x0F;

        gdt_entries[i].granularity |= gran & 0xF0;
        gdt_entries[i].access = access;
    }

    extern "C" void gdt_flush(uint32_t);

    void init(){
        printk::log(printk::INFO, "Initializing GDT...");

        gdt_ptr.limit = sizeof(gdt_entries) - 1;
        gdt_ptr.base = (uint32_t)&gdt_entries;

        // null descriptor
        set_entry(0, 0, 0, 0, 0);

        // code segment
        set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

        // data segment
        set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

        gdt_flush((uint32_t)&gdt_ptr);

        printk::log(printk::INFO, "GDT initialized");
    }
}