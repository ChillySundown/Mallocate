#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstddef>
#include "MetaArena.h"
#include "PageHeap.h"
#include "PageMap.h"
//Given a size in bytes, returns the size rounded up to the nearest power of align_up

MetaArena& meta_arena();
PageMap& page_map();
PageHeap& page_heap();
size_t align_up(size_t bytes, size_t align_up);

#endif