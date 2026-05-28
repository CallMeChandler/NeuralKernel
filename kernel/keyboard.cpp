#include "keyboard.h"
#include "printk.h"
#include "io.h"
#include "terminal.h"
#include "shell.h"

namespace keyboard
{
    static const char scancode_table[] =
    {
        0,
        27,
        '1','2','3','4','5','6','7','8','9','0',
        '-','=',
        '\b',
        '\t',

        'q','w','e','r','t','y','u','i','o','p',
        '[',']',

        '\n',

        0,

        'a','s','d','f','g','h','j','k','l',
        ';','\'','`',

        0,

        '\\',

        'z','x','c','v','b','n','m',
        ',', '.', '/',

        0,

        '*',

        0,

        ' '
    };



    extern "C"
    void keyboard_handler()
    {
        uint8_t scancode =
            inb(0x60);

        // ignore key releases
        if(scancode & 0x80)
        {
            outb(0x20,0x20);
            return;
        }

        char c =
            scancode_table[scancode];

        if(c)
        {
            shell::handle_input(c);
        }

        outb(0x20,0x20);
    }



    void init()
    {
        printk::log(
            printk::INFO,
            "Keyboard initialized"
        );
    }
}