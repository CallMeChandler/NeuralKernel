#include "elf.h"

namespace elf
{
    bool validate(
        const ELFHeader* header)
    {
        return
            header->magic[0] == 0x7F &&
            header->magic[1] == 'E' &&
            header->magic[2] == 'L' &&
            header->magic[3] == 'F';
    }
}