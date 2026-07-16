#pragma once

#include <stdint.h>

namespace vfs
{
    struct FileEntry
    {
        char name[64];
        uint32_t offset;
        uint32_t size;
    } __attribute__((packed));

    struct Header
    {
        char magic[4];
        uint32_t file_count;
    } __attribute__((packed));

    void init(void *initrd_start, uint32_t size);
    const void *open(const char *name, uint32_t *size_out);
    void list();
}
