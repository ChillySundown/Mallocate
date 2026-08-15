#include "PageMap.h"

int getPageRootIdx(int page_id) {
    return page_id >> (PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS) & (PAGEMAP_ROOT_SIZE-1);
}

int getPageBranchIdx(int page_id) {
    return page_id >> (PAGEMAP_LEAF_BITS) & (PAGEMAP_LEAF_SIZE-1);
}

int getPageLeafIdx(int page_id) {
    return page_id & (PAGEMAP_LEAF_SIZE-1);
}

void PageMap::init_arena(MetaArena* arena) {
    mem_arena = arena;
}

Span* PageMap::get(int page_id) {
    int root_idx = getPageRootIdx(page_id);
    int branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root->arr[root_idx];
    if(!branch_entry) {return nullptr;}
    auto* leaf_entry = branch_entry->arr[branch_idx];
    if(!leaf_entry) {return nullptr;}
    return leaf_entry->arr[leaf_idx];

}

void PageMap::set(int page_id, Span* span) {
    int root_idx = getPageRootIdx(page_id);
    int branch_idx = getPageBranchIdx(page_id);
    int leaf_idx = getPageLeafIdx(page_id);

    auto* branch_entry = map_root->arr[root_idx];
    auto* leaf_entry  = branch_entry->arr[branch_idx];
    leaf_entry->arr[leaf_idx] = span;
}

bool PageMap::ensure(int start_page, int length) {
    assert(mem_arena != nullptr);
    int root_idx;
    int branch_idx;
    int leaf_idx;
    for(int idx = start_page; idx < start_page + length; idx++) {
           root_idx = getPageRootIdx(idx);
           branch_idx = getPageBranchIdx(idx);
           leaf_idx = getPageLeafIdx(idx);

           auto* root = map_root->arr[root_idx];
           if(!root) {map_root->arr[root_idx] = static_cast<PageMapBranch*>(mem_arena->allocate(sizeof(PageMapBranch)));}


    }
}