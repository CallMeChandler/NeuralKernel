#pragma once
#include <stdint.h>
#include <stddef.h>

namespace terminal
{
    enum Color
    {
        BLACK = 0, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GREY,
        DARK_GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED,
        LIGHT_MAGENTA, LIGHT_BROWN, WHITE
    };

    void initialize();
    void clear();
    void set_color(uint8_t fg, uint8_t bg = BLACK);
    uint8_t get_color();
    void putchar(char c);
    void write(const char* str);
    void write_colored(const char* str, uint8_t fg, uint8_t bg = BLACK);
    void set_cursor(size_t row, size_t col);
    size_t get_row();
    size_t get_col();
}
