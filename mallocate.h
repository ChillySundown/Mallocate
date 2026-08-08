#include <cstddef>
#include <print>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
struct Block {
    size_t size {};
    bool free {};
    Block* prev {nullptr};
    Block* next {nullptr};
}; 

struct Span {

};

struct FreeBlock {
    FreeBlock* next {nullptr};
};

constexpr int size_classes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
constexpr size_t MAX_ALLOC_CLASS = static_cast<size_t>(size_classes[7]);

size_t align_up(size_t bytes, size_t align_up);
void* meta_malloc(size_t bytes);
void* mallocate(size_t bytes);
void deallocate(void* ptr);
