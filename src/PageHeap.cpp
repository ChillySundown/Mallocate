#include "PageHeap.h"
#include "globals.h"

void PageHeap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

Span* PageHeap::popFreeSpan() {
    if(!free_spans) {
        return static_cast<Span*>(mem_arena->allocate(sizeof(Span)));
    }
    Span* s = free_spans;
    free_spans = free_spans->next;
    free_spans->prev = nullptr;
    return s;
}

//Pushes a retired span onto the free list;
void PageHeap::pushFreeSpan(Span* s) {
    if(free_spans) {
        free_spans->prev = s;
    }
    s->starting_page_id = 0;
    s->num_pages = 0;

    s->prev = nullptr;
    s->next = free_spans;
    free_spans = s;

}


//Pushes a Span onto free_list[index]
void PageHeap::pushPages(size_t page_size, Span* s) {
    assert(page_size != 0);
    size_t index = std::min(page_size-1, static_cast<size_t>(255));
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

    Span* new_span = popFreeSpan();
    if(!new_span) {
        munmap(fresh_mem, req_bytes);
        return false;
    }

    bool ensure_pagemap = pm->ensure(start, length);
    if(!ensure_pagemap) {return false;}

    new_span->starting_page_id = start;
    new_span->num_pages = length;
    //All spans default state is SpanState::FREE

    for(size_t idx = start; idx < (start + length); idx++) {
        pm->set(idx, new_span);
    }
    pushPages(new_span->num_pages, new_span);
    return true;
}

//Given an index in the pageheap, either pop a one-page span or carve a smaller span from a larger page
Span* PageHeap::popPages(size_t index, size_t page_length) {
    //Assume that s->num_pages will never be smaller than page_length
    Span* s = free_page_lists[index];
    PageMap& global_map = page_map();

    //If num_pages is free, return the span
    if(s->num_pages == page_length) {
        free_page_lists[index] = free_page_lists[index]->next;
        free_page_lists[index]->prev = nullptr;
        return s;
    }

    //If greater page size than requested, carve from span and return new span
    Span* new_span = popFreeSpan();
    if(!new_span) {return nullptr;}
    new_span->starting_page_id = (s->starting_page_id + s->num_pages) - page_length;
    new_span->num_pages = page_length;

    //Maps each page in the span to the new_span
    for(size_t idx = new_span->starting_page_id; idx < new_span->starting_page_id + new_span->num_pages; idx++) {
        global_map.set(new_span->starting_page_id, new_span);
    }
    
    //Relocate span to new region
    free_page_lists[index] = free_page_lists[index]->next;
    s->num_pages -= page_length; 
    pushPages(s->num_pages, s);

    //s->starting_page_id -= page_length;
    return new_span;
}

void PageHeap::unlinkPages(Span* s) {
    assert(s != nullptr);
    size_t idx = s->num_pages-1;
    if(s == free_page_lists[idx]) {
        free_page_lists[idx] = s->next; //Uhh what if we keep moving head forward and have memory leak
    } else {
        Span* prev_span = s->prev;
        prev_span->next = s->next;
    }
}

void PageHeap::retireSpan(Span* s) {
    s->status = SpanState::FREE;
    pushFreeSpan(s);
}

void PageHeap::mergeSpans(Span* s, Span* r) {
    if(!s || !r) {return;}
    size_t start = r->starting_page_id;
    size_t len = r->num_pages;

    unlinkPages(s);
    s->num_pages += r->num_pages;
    //retire r
    for(size_t idx = start; idx < start + len; idx++) {
        pm->set(idx, s);
    }
}


Span* PageHeap::pageAlloc(size_t page_size) {
    size_t size_index = page_size - 1;
    if(size_index < 1) {return nullptr;}
    else if(size_index > 255) { size_index = 255;} //List of large pages
    
    if(free_page_lists[size_index]) { //First: try to pop from respective page list
        return popPages(size_index, page_size);
    } else {
        while(size_index < 255) { //Second: Iterate through all larger page lists until a free page is found
            if(free_page_lists[size_index] != nullptr) {
                return popPages(size_index, page_size);
            } else {
                size_index += 1;
            }
        }
    }

    //If the pageheap is empty
    if(!refillPageHeap(page_size)) {
        return nullptr;
    } else {
        return pageAlloc(page_size); //Recursion might give us some trouble
    }
     //Temporary -- //Third: Allocate a chunk of memory that can be sliced and diced for pages

}

void PageHeap::pageFree(Span* pages) {
    //Check
    //Set pages->state to FREE eventually
    if(!pages) {return;}
    pages->status = SpanState::FREE;

    size_t start = pages->starting_page_id;
    size_t len = pages->num_pages;

    Span* current = pages;
    if(pages->starting_page_id > 0) {
        size_t prev_page_id = pages->starting_page_id - 1;
        Span* prev_pages = pm->get(prev_page_id);

        //If prev_page exists and is ALSO free, merge pages
        if(prev_pages && prev_pages->status == SpanState::FREE) {
            mergeSpans(prev_pages, pages);
            current = prev_pages;
        }
    }

    Span* next_pages = pm->get(current->starting_page_id + current->num_pages); 
    if(next_pages && next_pages->status == SpanState::FREE) {
        mergeSpans(current, next_pages);
    }
    pushPages(current->num_pages, current);
        //Do I need to call e
}