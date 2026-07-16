#pragma once
#include <stdint.h>
namespace editor { bool open(const char*); bool active(); void handle_char(char); void handle_special(uint8_t); }
