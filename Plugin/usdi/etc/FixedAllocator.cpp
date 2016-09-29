#include "pch.h"
#include "usdiInternal.h"
#include "FixedAllocator.h"

namespace usdi {


struct Bin
{
    Bin *next;
};

FixedAllocator::FixedAllocator(size_t bin_size, size_t block_size)
    : m_bin_size(bin_size), m_block_size(block_size)
{
}

void* FixedAllocator::Allocate()
{
    lock_t lock(m_mutex);
    if (!m_head) {
        m_head = AlignedMalloc(m_bin_size * m_block_size, 0x20);
        auto *c = (char*)m_head;
        for (size_t i = 0; i < m_block_size - 1; ++i) {
            ((Bin*)(c + m_bin_size*i))->next = (Bin*)(c + m_bin_size*(i + 1));
        }
        ((Bin*)(c + (m_bin_size*(m_block_size - 1))))->next = nullptr;
        m_capacity += m_block_size;
    }

    ++m_in_use;
    void *ret = m_head;
    m_head = ((Bin*)m_head)->next;
    return ret;
}

void FixedAllocator::Free(void *addr)
{
    if (!addr) { return; }

    lock_t lock(m_mutex);
    ((Bin*)addr)->next = (Bin*)m_head;
    m_head = addr;
    --m_in_use;
}



static VtFixedAllocator g_allocators[] = {
    { 0x8, 1 },
    { 0x8, 1 },
    { 0x8, 1 },
    { 0x8, 1 },
    { 0x10, 512 }, // 4
    { 0x20, 512 },
    { 0x40, 512 },
    { 0x80, 512 },
    { 0x100, 128 }, // 8
    { 0x200, 128 },
    { 0x400, 128 },
    { 0x800, 128 },
    { 0x1000, 32 },
    { 0x2000, 32 },
    { 0x4000, 32 },
    { 0x8000, 32 },
    { 0x10000, 8 }, // 16
    { 0x20000, 8 },
    { 0x40000, 8 },
    { 0x80000, 8 },
    { 0x100000, 1 },
    { 0x200000, 1 },
    { 0x400000, 1 },
    { 0x800000, 1 },
    { 0x1000000, 1 }, // 24
    { 0x2000000, 1 },
    { 0x4000000, 1 },
    { 0x8000000, 1 },
    { 0x10000000, 1 },
    { 0x20000000, 1 },
    { 0x40000000, 1 },
    { 0x80000000, 1 },
    { 0x100000000, 1 }, // 32
    { 0x200000000, 1 },
    { 0x400000000, 1 },
    { 0x800000000, 1 },
    { 0x1000000000, 1 }, // 36
};

static inline size_t _SizeToIndex(size_t size)
{
    unsigned long index;
    _BitScanReverse64(&index, size);
    return index + (__popcnt64(size) > 1 ? 1 : 0);
}

void* FixedMalloc(size_t size)
{
    return g_allocators[_SizeToIndex(size)].Allocate();
}

void FixedFree(size_t size, void *addr)
{
    return g_allocators[_SizeToIndex(size)].Free(addr);
}

} // namespace usdi
