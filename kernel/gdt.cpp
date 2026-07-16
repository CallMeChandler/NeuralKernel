#include "gdt.h"
#include "printk.h"

namespace gdt
{
    struct Entry
    {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t granularity;
        uint8_t base_high;
    } __attribute__((packed));

    struct Pointer
    {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    struct TSS
    {
        uint32_t previous_tss;
        uint32_t esp0;
        uint32_t ss0;
        uint32_t esp1;
        uint32_t ss1;
        uint32_t esp2;
        uint32_t ss2;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        uint32_t es;
        uint32_t cs;
        uint32_t ss;
        uint32_t ds;
        uint32_t fs;
        uint32_t gs;
        uint32_t ldt;
        uint16_t trap;
        uint16_t iomap_base;
    } __attribute__((packed));

    static Entry gdt_entries[6];
    static Pointer gdt_ptr;
    static TSS tss;

    extern "C" void gdt_flush(uint32_t pointer);
    extern "C" void tss_flush();

    static void zero_memory(void *destination, uint32_t size)
    {
        uint8_t *bytes = (uint8_t *)destination;

        for (uint32_t i = 0; i < size; i++)
        {
            bytes[i] = 0;
        }
    }

    static void set_entry(
        int index,
        uint32_t base,
        uint32_t limit,
        uint8_t access,
        uint8_t granularity)
    {
        gdt_entries[index].base_low = base & 0xFFFF;
        gdt_entries[index].base_middle = (base >> 16) & 0xFF;
        gdt_entries[index].base_high = (base >> 24) & 0xFF;

        gdt_entries[index].limit_low = limit & 0xFFFF;
        gdt_entries[index].granularity = (limit >> 16) & 0x0F;
        gdt_entries[index].granularity |= granularity & 0xF0;
        gdt_entries[index].access = access;
    }

    void set_kernel_stack(uint32_t stack_top)
    {
        tss.esp0 = stack_top;
        tss.ss0 = 0x10;
    }

    void init()
    {
        printk::log(printk::INFO, "Initializing GDT...");

        gdt_ptr.limit = sizeof(gdt_entries) - 1;
        gdt_ptr.base = (uint32_t)&gdt_entries;

        set_entry(0, 0, 0, 0, 0);
        set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
        set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
        set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
        set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data

        zero_memory(&tss, sizeof(TSS));
        tss.ss0 = 0x10;
        tss.iomap_base = sizeof(TSS);

        set_entry(
            5,
            (uint32_t)&tss,
            sizeof(TSS) - 1,
            0x89,
            0x00);

        gdt_flush((uint32_t)&gdt_ptr);
        tss_flush();

        printk::log(printk::INFO, "GDT and TSS initialized");
    }
}
