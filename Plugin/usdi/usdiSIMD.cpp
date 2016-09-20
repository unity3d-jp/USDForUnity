#include "pch.h"
#include "usdiInternal.h"
#include "usdiSIMD.h"

namespace usdi {

void* AlignedMalloc(size_t size, size_t alignment)
{
    size_t mask = alignment - 1;
    size = (size + mask) & (~mask);
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void *ret;
    return posix_memalign(&ret, alignment, size) == 0 ? ret : nullptr;
#endif
}

void AlignedFree(void *addr)
{
#ifdef _WIN32
    _aligned_free(addr);
#else
    free(addr);
#endif
}

}
