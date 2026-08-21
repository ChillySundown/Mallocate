#include "PageHeap.h"
#include "globals.h"

void PageHeap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

void PageHeap::insertPages(size_t index, Span* s) {
    s->next = free_page_lists[index]; //Might cause index error
    free_page_lists[index]->prev = s;
    free_page_lists[index] = s;
}

bool PageHeap::refillPageHeap(size_t page_size) {
    size_t req_bytes = std::max(page_size * K_PAGE_SIZE, PAGEHEAP_REFILL_SIZE);
    void* fresh_mem = mmap(nullptr, req_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(fresh_mem == MAP_FAILED) {return false;}

    size_t start = reinterpret_cast<uintptr_t>(fresh_mem) >> K_PAGE_SHIFT;
    size_t length = req_bytes >> K_PAGE_SHIFT;

    Span* new_span = static_cast<Span*>(mem_arena->allocate(sizeof(Span)));
    if(!new_span) {
        munmap(fresh_mem, req_bytes);
        return false;
    }

    PageMap& global_map = page_map();
    bool ensure_pagemap = global_map.ensure(start, length);
    if(!ensure_pagemap) {return false;}

    new_span->starting_page_id = start;
    new_span->num_pages = length;
    //All spans default state is SpanState::FREE

    for(size_t idx = start; idx < (start + length); idx++) {
        global_map.set(idx, new_span);
    }
    
    size_t reloc_index = std::min(new_span->num_pages - 1, static_cast<size_t>(255));
    insertPages(reloc_index, new_span);
    return true;
}

//Given an index in the pageheap, either pop a one-page span or carve a smaller span from a larger page
Span* PageHeap::popPages(size_t index, size_t page_length) {
    //Assume that s->num_pages will never be smaller than page_length
    Span* s = free_page_lists[index];
    PageMap& global_map = page_map();
    Span* new_span = static_cast<Span*>(mem_arena->allocate(sizeof(Span)));
    if(!new_span) {return nullptr;}
    new_span->starting_page_id = (s->starting_page_id + s->num_pages) - page_length;
    new_span->num_pages = page_length;

    //Maps each page in the span to the new_span
    for(size_t idx = new_span->starting_page_id; idx < new_span->starting_page_id + new_span->num_pages; idx++) {
        global_map.set(new_span->starting_page_id, new_span);
    }
    
    free_page_lists[index] = s->next;
    s->num_pages -= page_length;
    size_t reloc_index = std::min(s->num_pages - 1, static_cast<size_t>(255));;
    insertPages(reloc_index, s);

    //s->starting_page_id -= page_length;
    return new_span;
}

Span* PageHeap::pageAlloc(size_t page_size) {
    size_t size_index = page_size - 1;
    if(size_index < 1) {return nullptr;}
    else if(size_index > 255) { size_index = 255;} //List of large pages
    
    if(free_page_lists[size_index]) { //First: try to pop from respective page list
        Span* s = free_page_lists[size_index];
        free_page_lists[size_index] = s->next;
        return s;

    } else {
        while(size_index < 255) { //Second: Iterate through all larger page lists until a free page is found
            if(free_page_lists[size_index] != nullptr) {
                return popPages(size_index, page_size);
            } else {
                size_index += 1;
            }
        }
    }

    if(!refillPageHeap(page_size)) {
        return nullptr;
    } else {
        return pageAlloc(page_size);
    }


     //Temporary -- //Third: Allocate a chunk of memory that can be sliced and diced for pages

}