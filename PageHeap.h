#ifndef PAGEHEAP_H
#define PAGEHEAP_H

#include "mallocate.h"
#include "PageMap.h"
class PageHeap {
    private:
        //Something something mutex
        Span* free_page_lists[256] {nullptr};
    public: 
        Span* pageAlloc(size_t num_pages);
        void pageFree(Span* pages, size_t length);
};

Span* popPages(Span* s, size_t page_length);

#endif