#pragma once

#include "tbb/spin_mutex.h"

namespace usdi {

class FixedAllocator
{
public:
    FixedAllocator(size_t bin_size, size_t block_size);
    void* Allocate();
    void Free(void *addr);

private:
    typedef tbb::spin_mutex mutex_t;
    typedef tbb::spin_mutex::scoped_lock lock_t;

    size_t m_bin_size = 0;
    size_t m_block_size = 0;
    size_t m_capacity = 0;
    size_t m_in_use = 0;
    void *m_head = nullptr;
    mutex_t m_mutex;
};

void* FixedMalloc(size_t size);
void  FixedFree(size_t size, void *addr);

} // namespace usdi