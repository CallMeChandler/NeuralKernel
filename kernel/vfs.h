#pragma once
#include <stdint.h>
namespace vfs {
struct FileEntry { char name[64]; uint32_t offset; uint32_t size; } __attribute__((packed));
struct Header { char magic[4]; uint32_t file_count; } __attribute__((packed));
void init(void*,uint32_t); const void* open(const char*,uint32_t*); bool write(const char*,const char*,uint32_t); bool remove(const char*); bool exists(const char*); bool valid_nkfs_name(const char*); void list(); }
