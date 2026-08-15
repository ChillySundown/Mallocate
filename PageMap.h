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

int getPageRootIdx(int page_id);
int getPageBranchIdx(int page_id);
int getPageLeafIdx(int page_id);

class PageMap {
    private:
        PageMapRoot* map_root;
        MetaArena* mem_arena {nullptr};
    public:
        void init_arena(MetaArena* arena);
        Span* get(int page_id); //Returns Span* of where page is store
        bool ensure(int start, int length);
        void set(int page_id, Span* span);
};

#endif
