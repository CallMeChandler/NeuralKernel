#include "elf.h"
#include "paging.h"

namespace elf
{
    static constexpr uint32_t PT_LOAD = 1;
    static constexpr uint32_t USER_MIN_ADDRESS = 0x00400000;
    static constexpr uint32_t USER_MAX_ADDRESS = 0x043FFFFF;

    static void copy_memory(void *destination, const void *source, uint32_t size)
    {
        uint8_t *to = (uint8_t *)destination;
        const uint8_t *from = (const uint8_t *)source;

        for (uint32_t i = 0; i < size; i++)
        {
            to[i] = from[i];
        }
    }

    static void zero_memory(void *destination, uint32_t size)
    {
        uint8_t *bytes = (uint8_t *)destination;

        for (uint32_t i = 0; i < size; i++)
        {
            bytes[i] = 0;
        }
    }

    bool validate(const void *image, uint32_t size)
    {
        if (image == nullptr || size < sizeof(Header))
        {
            return false;
        }

        const Header *header = (const Header *)image;

        if (header->ident[0] != 0x7F ||
            header->ident[1] != 'E' ||
            header->ident[2] != 'L' ||
            header->ident[3] != 'F')
        {
            return false;
        }

        if (header->ident[4] != 1 || // ELFCLASS32
            header->ident[5] != 1 || // little endian
            header->ident[6] != 1 ||
            header->type != 2 ||     // ET_EXEC
            header->machine != 3 ||  // EM_386
            header->program_header_entry_size != sizeof(ProgramHeader))
        {
            return false;
        }

        uint32_t table_size =
            (uint32_t)header->program_header_count *
            header->program_header_entry_size;

        if (header->program_header_offset > size ||
            table_size > size - header->program_header_offset)
        {
            return false;
        }

        return true;
    }

    bool load(
        const void *image,
        uint32_t size,
        uint32_t *entry_out)
    {
        if (!validate(image, size) || entry_out == nullptr)
        {
            return false;
        }

        const uint8_t *bytes = (const uint8_t *)image;
        const Header *header = (const Header *)image;

        for (uint32_t i = 0; i < header->program_header_count; i++)
        {
            const ProgramHeader *program =
                (const ProgramHeader *)(
                    bytes +
                    header->program_header_offset +
                    i * header->program_header_entry_size);

            if (program->type != PT_LOAD)
            {
                continue;
            }

            if (program->memory_size < program->file_size ||
                program->offset > size ||
                program->file_size > size - program->offset)
            {
                return false;
            }

            if (program->virtual_address < USER_MIN_ADDRESS ||
                program->virtual_address > USER_MAX_ADDRESS)
            {
                return false;
            }

            uint32_t segment_end =
                program->virtual_address + program->memory_size;

            if (segment_end < program->virtual_address ||
                segment_end > USER_MAX_ADDRESS + 1)
            {
                return false;
            }

            if (!paging::make_user_accessible(
                    program->virtual_address,
                    program->memory_size))
            {
                return false;
            }

            copy_memory(
                (void *)program->virtual_address,
                bytes + program->offset,
                program->file_size);

            zero_memory(
                (void *)(program->virtual_address + program->file_size),
                program->memory_size - program->file_size);
        }

        if (header->entry < USER_MIN_ADDRESS ||
            header->entry > USER_MAX_ADDRESS)
        {
            return false;
        }

        *entry_out = header->entry;
        return true;
    }
}
