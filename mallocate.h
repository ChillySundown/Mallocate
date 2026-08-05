#include <cstddef>
#include <print>
#include <iostream>
#include <sys/mman.h>
struct Block {
    size_t size {};
    bool free {};
    Block* prev {nullptr};
    Block* next {nullptr};
}; 
void* mallocate(size_t bytes);
void deallocate(void* ptr);
