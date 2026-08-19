#include "PageHeap.h"

Span* pop_pages(Span* s, size_t page_length) {
    //Assume that s->num_pages will never be smaller than page_length
    if(s->num_pages == page_length) {
        return s;
    } else {
        Span new_span;
        new_span.starting_page_id = s->starting_page_id + page_length;
        new_span.num_pages = page_length;

        s->num_pages -= page_length;
        s->starting_page_id += page_length;
        //s->starting_page_id -= page_length;
        return &new_span;
    }
}

Span* PageHeap::pageAlloc(size_t page_size) {
    size_t size_index = page_size - 1;
    if(size_index < 1) {return nullptr;}
    else if(size_index > 255) { size_index = 255;} //List of large pages
    
    if(free_page_lists[size_index]) {
        return pop_pages(free_page_lists[size_index], page_size);
    } else {
        while(size_index < 255) {
            if(free_page_lists[size_index] != nullptr) {
                return pop_pages(free_page_lists[size_index], page_size);
            } else {
                size_index += 1;
            }
        }

    }
}