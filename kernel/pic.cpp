#include "pic.h"
#include "io.h"
#include "printk.h"

namespace pic
{
    void remap(){
        printk::log(
            printk::INFO,
            "Remapping PIC..."
        );

        outb(0x20,0x11);
        outb(0xA0,0x11);

        outb(0x21,32);
        outb(0xA1,40);

        outb(0x21,0x04);
        outb(0xA1,0x02);

        outb(0x21,0x01);
        outb(0xA1,0x01);

        outb(0x21,0);
        outb(0xA1,0);

        printk::log(
            printk::INFO,
            "PIC ready"
        );
    }
}