#pragma once

#include <stdint.h>

namespace elf
{
    struct ELFHeader
    {
        unsigned char magic[4];

        uint8_t class_type;

        uint8_t endian;

        uint8_t version;

        uint8_t abi;

        uint8_t padding[8];

        uint16_t type;

        uint16_t machine;

        uint32_t version2;

        uint32_t entry;
    };

    bool validate(
        const ELFHeader* header
    );
}