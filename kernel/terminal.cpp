#include "terminal.h"

namespace terminal
{
    static uint16_t* buffer = (uint16_t*)0xB8000;

    static size_t row = 0;
    static size_t col = 0;

    static uint8_t color = 0x07;

    static uint16_t make_entry(char c, uint8_t color)
    {
        return (uint16_t)c | (uint16_t)color << 8;
    }

    void clear()
    {
        for (int i = 0; i < 80 * 25; i++)
        {
            buffer[i] = make_entry(' ', color);
        }

        row = 0;
        col = 0;
    }

    void initialize()
    {
        clear();
    }

    void set_color(uint8_t fg, uint8_t bg)
    {
        color = fg | (bg << 4);
    }

    void putchar(char c)
    {
        if (c == '\n')
        {
            col = 0;
            row++;
        }
        else
        {
            buffer[row * 80 + col] = make_entry(c, color);
            col++;
        }

        // wrap line
        if (col >= 80)
        {
            col = 0;
            row++;
        }

        // scroll screen
        if (row >= 25)
        {
            for (int i = 80; i < 80 * 25; i++)
            {
                buffer[i - 80] = buffer[i];
            }

            for (int i = 80 * 24; i < 80 * 25; i++)
            {
                buffer[i] = make_entry(' ', color);
            }

            row = 24;
        }
    }

    void write(const char* str)
    {
        for (int i = 0; str[i] != '\0'; i++)
        {
            putchar(str[i]);
        }
    }
}