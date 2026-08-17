#ifndef PAGEMAP_H
#define PAGEMAP_H

#include "mallocate.h"
#include <cassert>
struct PageMapLeaf {
    Span* arr[PAGEMAP_LEAF_SIZE];
};

struct PageMapBranch {
    PageMapLeaf* arr[PAGEMAP_BRANCH_SIZE];
};

struct PageMapRoot {
    PageMapBranch* arr[PAGEMAP_ROOT_SIZE];
};

int getPageRootIdx(size_t page_id);
int getPageBranchIdx(size_t page_id);
int getPageLeafIdx(size_t page_id);

class PageMap {
    private:
        PageMapRoot map_root {};
        MetaArena* mem_arena {nullptr};
    public:
        void init_arena(MetaArena* arena);
        Span* get(size_t page_id); //Returns Span* of where page is store
        bool ensure(size_t start, size_t length);
        void set(size_t page_id, Span* span);
        size_t getBytesAllocated();
        size_t getSizeClass(size_t page_id);
};

#endif
