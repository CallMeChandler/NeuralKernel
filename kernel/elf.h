#pragma once

#include <stdint.h>

namespace elf
{
    struct Header
    {
        uint8_t ident[16];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint32_t entry;
        uint32_t program_header_offset;
        uint32_t section_header_offset;
        uint32_t flags;
        uint16_t header_size;
        uint16_t program_header_entry_size;
        uint16_t program_header_count;
        uint16_t section_header_entry_size;
        uint16_t section_header_count;
        uint16_t section_name_index;
    } __attribute__((packed));

    struct ProgramHeader
    {
        uint32_t type;
        uint32_t offset;
        uint32_t virtual_address;
        uint32_t physical_address;
        uint32_t file_size;
        uint32_t memory_size;
        uint32_t flags;
        uint32_t alignment;
    } __attribute__((packed));

    bool validate(const void *image, uint32_t size);

    bool load(
        const void *image,
        uint32_t size,
        uint32_t *entry_out);
}
