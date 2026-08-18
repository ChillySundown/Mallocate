#include "PageHeap.h"

Span* PageHeap::pageAlloc(size_t page_size) {
    size_t size_index = page_size - 1;
    if(size_index < 1) {return nullptr;}
    else if(size_index > 255) { size_index = 255;} //List of large pages
    
    if(!free_page_lists[size_index]) {
        free_page_lists[size_index];//= mmap
    }
}