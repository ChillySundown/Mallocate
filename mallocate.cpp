#include <cstddef>
#include "mallocate.h"

constexpr size_t HEAP_SIZE {1024 * 1024}; //Represents the size of our heap in bytes
alignas(8) static unsigned char heap[HEAP_SIZE]; //Our actual pool of memory
static unsigned char* heap_ptr = heap; //pointer to our heap

Block* heap_head {nullptr};

void init_heap() {
   heap_head = reinterpret_cast<Block*>(heap_ptr); //Intializes our block linked list with ptr from our heap array
   heap_head->free = true;
   heap_head->next = nullptr;
   heap_head->size = HEAP_SIZE - sizeof(Block);
}

//Aligns bytes upward by given power
size_t align_up(size_t bytes, size_t alignment) {
    return (bytes + (alignment-1)) & ~(alignment - 1);
}

void split_free_block(Block* ptr, size_t split_size) {
    //Steps into memory payload, moves by split_size bytes, and creates a new header
    auto* new_block {reinterpret_cast<Block*>(reinterpret_cast<unsigned char*>(ptr + 1) + split_size)};

    new_block->size = new_block->size - split_size - sizeof(Block*);
    new_block->free = true;
    new_block->next = ptr;
}

void* mallocate(size_t bytes) {
    if(!heap_head) {
        init_heap();
    } else if(bytes == 0) {
        return nullptr;
    }

    bytes = align_up(bytes, alignof(double));

    auto* w = heap_head;
    while(w) {
        if(w->free && w->size >= bytes) {
            split_free_block(w, bytes);
            w->free = false;
            return reinterpret_cast<void*>(w);
        }

        w = w->next;
    }
}
void deallocate(void* ptr) {

}


