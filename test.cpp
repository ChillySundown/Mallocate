#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "mallocate.h"
#include <cmath>

/*
Metadata Malloc tests
-Alignment Test
-Memory Overlap Test
*/

TEST_CASE("Testing Meta Malloc byte alignment") {
    MetaArena meta_space;
    for(int i = 1; i < 1024; i += 3) {
        void* p = meta_space.allocate(i);
        REQUIRE(p != nullptr);
        CHECK(reinterpret_cast<uintptr_t>(p) % alignof(max_align_t) == 0);
    }
};

TEST_CASE("Testing Meta Mallocs no memory overlap") {
    MetaArena meta_space;
    std::vector<std::pair<uintptr_t, size_t>> allocs;
    for(int i = 0; i < 2000; i++) {
        size_t alloc_size = 1 + (rand() % 200); //Selects random size from 1 to 200 bytes
        void* ptr = meta_space.allocate(alloc_size);
        REQUIRE(ptr != nullptr);
        allocs.push_back({reinterpret_cast<uintptr_t>(ptr), alloc_size});
    }

    std::sort(allocs.begin(), allocs.end());

    for(int i = 1; i < allocs.size(); i++) {
        CHECK(allocs[i-1].first + allocs[i-1].second <= allocs[i].first);
    }
}