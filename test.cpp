#include "doctest.h"
#include "mallocate.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

TEST_CASE("Testing Meta Malloc byte alignment") {
    MetaArena meta_space;
    for(int i = 1; i < 1024; i += 3) {
        void* p = meta_space.allocate(i);
        CHECK(reinterpret_cast<uintptr_t>(p) % alignof(max_align_t) == 0);
    }
};