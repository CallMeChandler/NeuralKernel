#include "vfs.h"
#include "printk.h"
#include "terminal.h"

namespace vfs
{
    static const uint8_t *initrd_base = nullptr;
    static uint32_t initrd_size = 0;
    static const Header *header = nullptr;

    static bool names_equal(const char *a, const char *b)
    {
        int i = 0;

        while (a[i] && b[i])
        {
            if (a[i] != b[i])
            {
                return false;
            }

            i++;
        }

        return a[i] == b[i];
    }

    void init(void *start, uint32_t size)
    {
        initrd_base = (const uint8_t *)start;
        initrd_size = size;
        header = nullptr;

        if (start == nullptr || size < sizeof(Header))
        {
            printk::log(printk::ERROR, "VFS: invalid initrd");
            return;
        }

        const Header *candidate = (const Header *)start;

        if (candidate->magic[0] != 'N' ||
            candidate->magic[1] != 'K' ||
            candidate->magic[2] != 'F' ||
            candidate->magic[3] != 'S')
        {
            printk::log(printk::ERROR, "VFS: bad magic");
            return;
        }

        uint32_t entries_size =
            candidate->file_count * sizeof(FileEntry);

        if (candidate->file_count > 64 ||
            entries_size > size - sizeof(Header))
        {
            printk::log(printk::ERROR, "VFS: corrupt file table");
            return;
        }

        header = candidate;
        printk::log(printk::INFO, "VFS: initrd mounted");
    }

    const void *open(const char *name, uint32_t *size_out)
    {
        if (header == nullptr || name == nullptr)
        {
            return nullptr;
        }

        const FileEntry *entries =
            (const FileEntry *)(initrd_base + sizeof(Header));

        for (uint32_t i = 0; i < header->file_count; i++)
        {
            if (!names_equal(entries[i].name, name))
            {
                continue;
            }

            if (entries[i].offset > initrd_size ||
                entries[i].size > initrd_size - entries[i].offset)
            {
                printk::log(printk::ERROR, "VFS: corrupt file entry");
                return nullptr;
            }

            if (size_out != nullptr)
            {
                *size_out = entries[i].size;
            }

            return initrd_base + entries[i].offset;
        }

        return nullptr;
    }

    void list()
    {
        if (header == nullptr)
        {
            terminal::write("VFS not mounted\n");
            return;
        }

        const FileEntry *entries =
            (const FileEntry *)(initrd_base + sizeof(Header));

        terminal::write("NKFS files:\n");

        for (uint32_t i = 0; i < header->file_count; i++)
        {
            terminal::write("  ");
            terminal::write(entries[i].name);
            terminal::write("\n");
        }
    }
}
