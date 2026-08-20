#include <cstddef>
struct MetaArena {
    unsigned char* meta_current {nullptr};
    unsigned char* meta_end {nullptr};
    size_t byte_count {0};

    void* allocate(size_t bytes);
};
