#include "PageHeap.h"
#include "globals.h"

void PageHeap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

Span* PageHeap::popFreeSpan() {
    if(!free_spans) {return nullptr;}
    Span* s = free_spans;
    free_spans = free_spans->next;
    free_spans->prev = nullptr;
    return s;
}

void PageHeap::pushFreeSpan(Span* s) {
    free_spans->prev = s;
    s->next = free_spans;
}


//Pushes a Span onto free_list[index]
void PageHeap::pushPages(size_t index, Span* s) {
    index = std::min(index, static_cast<size_t>(255));
    s->next = free_page_lists[index]; //Might cause index error
    free_page_lists[index]->prev = s;
    free_page_lists[index] = s;
}

//Refills page heap
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
    
    size_t reloc_index = new_span->num_pages - 1;
    pushPages(reloc_index, new_span);
    return true;
}

//Given an index in the pageheap, either pop a one-page span or carve a smaller span from a larger page
Span* PageHeap::popPages(size_t index, size_t page_length) {
    //Assume that s->num_pages will never be smaller than page_length
    Span* s = free_page_lists[index];
    PageMap& global_map = page_map();

    //If num_pages is free, return the span
    if(s->num_pages == page_length) {return s;}

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
    size_t reloc_index = s->num_pages - 1;
    pushPages(reloc_index, s);

    //s->starting_page_id -= page_length;
    return new_span;
}

void PageHeap::unlinkPages(Span* s) {
    assert(s == nullptr);
    size_t idx = s->num_pages-1;
    if(s == free_page_lists[idx]) {
        free_page_lists[idx] = s->next; //Uhh what if we keep moving head forward and have memory leak
    } else {
        Span* prev_span = s->prev;
        prev_span->next = s->next;
    }
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

void PageHeap::pageFree(Span* pages) {
    //Check
    //Set pages->state to FREE eventually
    PageMap& global_map = page_map();

    size_t start = pages->starting_page_id;
    size_t len = pages->num_pages;

    Span* current = pages;
    pages->status = SpanState::FREE;
    if(pages->starting_page_id > 0) {
        size_t prev_page_id = pages->starting_page_id - 1;
        Span* prev_pages = global_map.get(prev_page_id);
        if(prev_pages && prev_pages->status == SpanState::FREE) {
            //Remove prev_page from free_list
            prev_pages->num_pages += pages->num_pages;
            for(size_t idx = start; idx < (start + len); idx++) {
                global_map.set(idx, prev_pages);
            }
            unlinkPages(prev_pages);
            unlinkPages(pages);
            current = prev_pages;
        }
    }

    Span* next_pages = global_map.get(current->starting_page_id + current->num_pages);
    start = next_pages->starting_page_id;
    len = next_pages->num_pages;
    if(next_pages && next_pages->status == SpanState::FREE) {
        current->num_pages += next_pages->num_pages;
        for(size_t idx = start; idx < (start + len); idx++) {
            global_map.set(idx, current);
        }
        unlinkPages(next_pages);
    }
    
    pushPages(current->num_pages - 1, current);
    //Do I need to call e
}