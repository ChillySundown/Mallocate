#include "mallocate.h"

constexpr size_t HEAP_SIZE {1024 * 1024}; //Represents the size of our heap in bytes
//alignas(8) static unsigned char heap[HEAP_SIZE]; //Our actual pool of memory
static unsigned char* heap_ptr = static_cast<unsigned char*>(mmap(nullptr, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0)); //pointer to our heap

FreeBlock* heap_cache[8] {nullptr};
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

    new_block->size = ptr->size - split_size - sizeof(Block);
    new_block->free = true;
    new_block->next = ptr->next;

    ptr->next = new_block;
    ptr->size = split_size;
}

void merge_free_blocks() {
    auto* current {heap_head};

    while(current && current->next) {
        if(current->free && current->next->free) {
            // Block* new_block {current};
            // new_block->size = current->size + current->next->size;
            // new_block->free = true;
            current->size += current->next->size + sizeof(Block);
            current->next = current->next->next;
        }
        current = current->next;
    }
}

void* mallocate(size_t bytes) {
    if(bytes == 0) { return nullptr; }
    if(!heap_head) { init_heap(); }
    bytes = align_up(bytes, alignof(double));

    auto* w = heap_head;
    while(w) {
        if(w->free && w->size >= bytes) {
            split_free_block(w, bytes);
            w->free = false;
            return w + 1; //Moves past header to payload
        }

        w = w->next;
    }
    return nullptr;
}
void deallocate(void* ptr) {
    if(!ptr) {
        return;
    }
    auto* new_header {reinterpret_cast<Block*>(ptr) - 1}; //Removes back by sizeof(Block) bytes and creates a new header
    new_header->free = true;
    merge_free_blocks();
}

void print_heap() {
    size_t i {};
    auto* current {heap_head};

    while(current) {
        std::println("[Block {}]: {} {} bytes", ++i, current->free ? "free" : "in use", current->size);
        current = current->next;
    }
}

int main() {
    auto* a {mallocate(10)};
    auto* b {mallocate(20)};
    print_heap();
    deallocate(a);
    deallocate(b);

    print_heap();
    auto* c {mallocate(30)};

    return 0;
}