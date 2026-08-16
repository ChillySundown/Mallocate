#include "PageMap.h"

int getPageRootIdx(size_t page_id) {
    return page_id >> (PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS) & (PAGEMAP_ROOT_SIZE-1);
}

int getPageBranchIdx(size_t page_id) {
    return page_id >> (PAGEMAP_LEAF_BITS) & (PAGEMAP_LEAF_SIZE-1);
}

int getPageLeafIdx(size_t page_id) {
    return page_id & (PAGEMAP_LEAF_SIZE-1);
}

size_t PageMap::getBytesAllocated() {
    if(mem_arena) {
        return mem_arena->byte_count;
    } else {
        return 0;
    }
}

void PageMap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

Span* PageMap::get(size_t page_id) {
    size_t root_idx = getPageRootIdx(page_id);
    size_t branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root->arr[root_idx];
    if(!branch_entry) {return nullptr;}
    auto* leaf_entry = branch_entry->arr[branch_idx];
    if(!leaf_entry) {return nullptr;}
    return leaf_entry->arr[leaf_idx];

}

void PageMap::set(size_t page_id, Span* span) {
    int root_idx = getPageRootIdx(page_id);
    int branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root->arr[root_idx];
    auto* leaf_entry  = branch_entry->arr[branch_idx];
    leaf_entry->arr[leaf_idx] = span;
}

bool PageMap::ensure(size_t start_page, size_t length) {
    assert(mem_arena != nullptr);
    int root_idx;
    int branch_idx;
    int leaf_idx;
    /*
    TODO: Make iteration chunked and remove unnecesary calculation of page indicies
    */
    for(int idx = start_page; idx < start_page + length; idx += PAGEMAP_LEAF_SIZE) {
           root_idx = getPageRootIdx(idx);
           branch_idx = getPageBranchIdx(idx);
           leaf_idx = getPageLeafIdx(idx);

           auto* root = map_root->arr[root_idx];
           if(!root) {
            map_root->arr[root_idx] = static_cast<PageMapBranch*>(mem_arena->allocate(sizeof(PageMapBranch)));
           }
           auto* branch = map_root->arr[root_idx];
           auto* leaf = branch->arr[branch_idx];
           if(!leaf) {
                branch->arr[branch_idx] = static_cast<PageMapLeaf*>(mem_arena->allocate(sizeof(PageMapLeaf)));
           }
    }
    return true;
}