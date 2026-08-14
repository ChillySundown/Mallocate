#include "PageMap.h"

Span* PageMap::get(int page_id) {
    int root_idx = page_id >> (PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS) & (PAGEMAP_ROOT_SIZE-1);
    int branch_idx = page_id >> (PAGEMAP_LEAF_BITS) & (PAGEMAP_LEAF_SIZE-1);
    int leaf_idx = page_id & (PAGEMAP_LEAF_SIZE - 1);

    auto* branch_entry = map_root.arr[root_idx];
    if(!branch_entry) {return nullptr;}
    auto* leaf_entry = branch_entry->arr[branch_idx];
    if(!leaf_entry) {return nullptr;}
    Span* entry = leaf_entry->arr[leaf_idx];

    return entry;
}