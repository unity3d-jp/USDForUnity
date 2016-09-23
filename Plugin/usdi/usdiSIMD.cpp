#include "pch.h"
#include "usdiInternal.h"
#include "usdiSIMD.h"

namespace usdi {

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

}
