#ifndef PAGEHEAP_H
#define PAGEHEAP_H

#include "mallocate.h"
#include "PageMap.h"
class PageHeap {
    private:
        //Something something mutex
        Span* popPages(size_t index, size_t page_length);
        Span* free_page_lists[256] {nullptr};
        MetaArena* mem_arena {nullptr};
    public: 
        void init_arena(MetaArena* arena);
        Span* pageAlloc(size_t num_pages);
        void pageFree(Span* pages, size_t length);
    
};

#endif