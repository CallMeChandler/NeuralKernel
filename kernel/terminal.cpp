#include "terminal.h"
#include "io.h"

namespace terminal
{
    static uint16_t* const buffer = (uint16_t*)0xB8000;
    static size_t row = 0;
    static size_t col = 0;
    static uint8_t color = 0x07;

    static uint16_t make_entry(char c, uint8_t attribute)
    {
        return (uint16_t)c | ((uint16_t)attribute << 8);
    }

    static void update_cursor()
    {
        uint16_t position = (uint16_t)(row * 80 + col);
        outb(0x3D4, 0x0F);
        outb(0x3D5, position & 0xFF);
        outb(0x3D4, 0x0E);
        outb(0x3D5, (position >> 8) & 0xFF);
    }

    void clear()
    {
        for (int i = 0; i < 80 * 25; i++)
            buffer[i] = make_entry(' ', color);
        row = 0;
        col = 0;
        update_cursor();
    }

    void initialize()
    {
        set_color(LIGHT_GREY, BLACK);
        clear();
    }

    void set_color(uint8_t fg, uint8_t bg)
    {
        color = (fg & 0x0F) | ((bg & 0x0F) << 4);
    }

    uint8_t get_color()
    {
        return color;
    }

    void putchar(char c)
    {
        if (c == '\b')
        {
            if (col > 0) col--;
            else if (row > 0) { row--; col = 79; }
            buffer[row * 80 + col] = make_entry(' ', color);
            update_cursor();
            return;
        }

        if (c == '\n') { col = 0; row++; }
        else if (c == '\r') col = 0;
        else { buffer[row * 80 + col] = make_entry(c, color); col++; }

        if (col >= 80) { col = 0; row++; }
        if (row >= 25)
        {
            for (int i = 80; i < 80 * 25; i++) buffer[i - 80] = buffer[i];
            for (int i = 80 * 24; i < 80 * 25; i++) buffer[i] = make_entry(' ', color);
            row = 24;
        }
        update_cursor();
    }

    void write(const char* str)
    {
        if (!str) return;
        for (int i = 0; str[i]; i++) putchar(str[i]);
    }

    void write_colored(const char* str, uint8_t fg, uint8_t bg)
    {
        uint8_t previous = color;
        set_color(fg, bg);
        write(str);
        color = previous;
    }

    void set_cursor(size_t new_row, size_t new_col)
    {
        row = new_row < 25 ? new_row : 24;
        col = new_col < 80 ? new_col : 79;
        update_cursor();
    }

    size_t get_row() { return row; }
    size_t get_col() { return col; }
}
