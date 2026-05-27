#include "idt.h"
#include "printk.h"

namespace idt
{
    struct Entry
    {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t zero;
        uint8_t flags;
        uint16_t offset_high;
    } __attribute__((packed));

    struct Pointer
    {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    static Entry idt[256];
    static Pointer idt_ptr;

    extern "C" void idt_load(uint32_t);

    static void set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
    {
        idt[num].offset_low = base & 0xFFFF;
        idt[num].selector = sel;
        idt[num].zero = 0;
        idt[num].flags = flags;
        idt[num].offset_high = (base >> 16);
    }

    extern "C" void isr0();

    extern "C" void irq0();

    extern "C" void irq1();

    void init()
    {
        printk::log(
            printk::INFO,
            "Initializing IDT...");

        for (int i = 0; i < 256; i++)
        {
            set_gate(
                i,
                0,
                0,
                0);
        }

        set_gate(
            0,
            (uint32_t)isr0,
            0x08,
            0x8E);

        set_gate(
            32,
            (uint32_t)irq0,
            0x08,
            0x8E);

        set_gate(
            33,
            (uint32_t)irq1,
            0x08,
            0x8E);

        idt_ptr.limit =
            sizeof(idt) - 1;

        idt_ptr.base =
            (uint32_t)&idt;

        idt_load(
            (uint32_t)&idt_ptr);

        printk::log(
            printk::INFO,
            "IDT initialized");
    }
}