#include <cstddef>
#include <print>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

enum class SpanState : uint8_t {FREE, IN_USE, LARGE_OBJ};

constexpr int K_PAGE_SHIFT {14};
constexpr int PAGEMAP_ROOT_BITS {12};
constexpr int PAGEMAP_BRANCH_BITS {9};
constexpr int PAGEMAP_LEAF_BITS {13};

static_assert(PAGEMAP_ROOT_BITS + PAGEMAP_BRANCH_BITS + PAGEMAP_LEAF_BITS == 34);

constexpr int PAGEMAP_ROOT_SIZE = 1ull << PAGEMAP_ROOT_BITS;
constexpr int PAGEMAP_BRANCH_SIZE = 1ull << PAGEMAP_BRANCH_BITS;
constexpr int PAGEMAP_LEAF_SIZE = 1ull << PAGEMAP_LEAF_BITS;

constexpr int size_classes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
constexpr size_t HEAP_SIZE {1024 * 1024}; //Represents the size of our heap in bytes (128 to be exact)
//alignas(8) static unsigned char heap[HEAP_SIZE]; //Our actual pool of memory
static const size_t page_size = sysconf(_SC_PAGESIZE);
constexpr size_t K_MAX_SIZE = static_cast<size_t>(size_classes[7]);

struct Block {
    size_t size {};
    bool free {};
    Block* prev {nullptr};
    Block* next {nullptr};
}; 

struct FreeBlock { //Similar to block, but size is already known by size_class
    FreeBlock* next {nullptr};
};

struct Span {
    size_t starting_page_id;
    size_t num_pages;
    Span* next;
    Span* prev;
    int8_t status;
    FreeBlock* objects;
    size_t size_class;
};

struct PageMapLeaf {
    Span* arr[PAGEMAP_LEAF_SIZE];
};

struct PageMapBranch {
    PageMapLeaf* arr[PAGEMAP_BRANCH_SIZE];
};

struct PageMapRoot {
    PageMapBranch* arr[PAGEMAP_ROOT_SIZE];
};




size_t align_up(size_t bytes, size_t align_up);
void* meta_malloc(size_t bytes);
void* mallocate(size_t bytes);
void deallocate(void* ptr);
size_t getPageID(void* pagePtr);
