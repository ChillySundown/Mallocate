#include <cstddef>
#ifndef META_ARENA_H
#define META_ARENA_H
struct MetaArena {
    unsigned char* meta_current {nullptr};
    unsigned char* meta_end {nullptr};
    size_t byte_count {0};

    void* allocate(size_t bytes);
};
#endif
