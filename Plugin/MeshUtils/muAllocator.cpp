#include "pch.h"
#include "muAllocator.h"

namespace mu {

void* AlignedMalloc(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size = (size + mask) & (~mask);
    return _mm_malloc(size, alignment);
}

void AlignedFree(void *addr)
{
    _mm_free(addr);
}

} // namespace mu