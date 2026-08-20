#include "MetaArena.h"
#include "constants.h"
#include "globals.h"

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