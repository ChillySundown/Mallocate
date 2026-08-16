#include "mallocate.h"
#include "PageMap.h"
static unsigned char* heap_ptr = static_cast<unsigned char*>(mmap(nullptr, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0)); //pointer to our heap
MetaArena meta_space; //Memory space for metadata

FreeBlock* frontend_cache[8]; 
Block* heap_head {nullptr};


void init_heap() {
   heap_head = reinterpret_cast<Block*>(heap_ptr); //Intializes our block linked list with ptr from our heap array
   heap_head->free = true;
   heap_head->next = nullptr;
   heap_head->size = HEAP_SIZE - sizeof(Block);
}

void* MetaArena::allocate(size_t bytes) {
    if(bytes >= (SIZE_MAX - 16)) { //Checks to see if memory is near alignment limit
        //std::cout << "CANNOT ALLOCATE - MEMORY SIZE AT ALIGNMENT LIMIT" << std::endl; Need a thread-safe error message
        return nullptr;
    }
    size_t aligned_bytes = align_up(bytes, alignof(std::max_align_t));
    size_t mapped_bytes = std::max(HEAP_SIZE, align_up(aligned_bytes, page_size));
    if(!meta_current || static_cast<size_t>(meta_end - meta_current) < aligned_bytes) {
        void* mapped_mem = mmap(nullptr, mapped_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if(mapped_mem == MAP_FAILED) {
            return nullptr;
        } else {
            meta_current = reinterpret_cast<unsigned char*>(mapped_mem);
            meta_end = meta_current + mapped_bytes; 
        }
    }

    if(static_cast<size_t>(meta_end - meta_current) >= aligned_bytes) {
            unsigned char* mem_start = meta_current;
            meta_current += aligned_bytes;
            byte_count += aligned_bytes;
            return mem_start;
    }
    return nullptr;
}

//Memory allocator for metadata
void* meta_malloc(size_t bytes) {
    return meta_space.allocate(bytes);
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

// int main() {
//     std::cout << "Size of pages on Apple Silicon: " << page_size << std::endl;
//     auto* a {mallocate(10)};
//     auto* b {mallocate(20)};
//     print_heap();
//     deallocate(a);
//     deallocate(b);

//     print_heap();
//     auto* c {mallocate(30)};

//     return 0;
// }