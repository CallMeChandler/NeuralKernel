#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char     magic[4];
    uint32_t file_count;
} Header;

typedef struct {
    char     name[64];
    uint32_t offset;
    uint32_t size;
} FileEntry;

int main(int argc, char* argv[]) {
    // argv: mkinitrd output.nkfs file1 name1 file2 name2 ...
    if (argc < 4 || (argc - 2) % 2 != 0) {
        fprintf(stderr,
            "Usage: mkinitrd out.nkfs file1 name1 [file2 name2 ...]\n");
        return 1;
    }

    int file_count = (argc - 2) / 2;

    // Read all input files
    uint8_t**  data  = malloc(file_count * sizeof(uint8_t*));
    uint32_t*  sizes = malloc(file_count * sizeof(uint32_t));
    char**     names = malloc(file_count * sizeof(char*));

    for (int i = 0; i < file_count; i++) {
        const char* path = argv[2 + i * 2];
        names[i]         = argv[3 + i * 2];

        FILE* f = fopen(path, "rb");
        if (!f) { fprintf(stderr, "Cannot open %s\n", path); return 1; }

        fseek(f, 0, SEEK_END);
        sizes[i] = ftell(f);
        rewind(f);

        data[i] = malloc(sizes[i]);
        fread(data[i], 1, sizes[i], f);
        fclose(f);
    }

    // Calculate offsets
    uint32_t data_start = sizeof(Header)
                        + file_count * sizeof(FileEntry);

    uint32_t* offsets = malloc(file_count * sizeof(uint32_t));
    uint32_t  cursor  = data_start;

    for (int i = 0; i < file_count; i++) {
        offsets[i]  = cursor;
        cursor     += sizes[i];
    }

    // Write archive
    FILE* out = fopen(argv[1], "wb");
    if (!out) { fprintf(stderr, "Cannot open output\n"); return 1; }

    Header h;
    memcpy(h.magic, "NKFS", 4);
    h.file_count = file_count;
    fwrite(&h, sizeof(h), 1, out);

    for (int i = 0; i < file_count; i++) {
        FileEntry e;
        memset(&e, 0, sizeof(e));
        strncpy(e.name, names[i], 63);
        e.offset = offsets[i];
        e.size   = sizes[i];
        fwrite(&e, sizeof(e), 1, out);
    }

    for (int i = 0; i < file_count; i++)
        fwrite(data[i], 1, sizes[i], out);

    fclose(out);
    printf("NKFS: wrote %d files to %s\n", file_count, argv[1]);
    return 0;
}