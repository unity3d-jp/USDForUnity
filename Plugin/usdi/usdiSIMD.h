#pragma once

#ifdef usdiEnableISPC
    #include "Vertex/VertexOperations.h"
#endif

namespace usdi {

void* AlignedMalloc(size_t size, size_t alignment);
void  AlignedFree(void *addr);

template<class T, size_t Alignment>
class AlignedAllocator
{
public:
    typedef T value_type;
    static const size_t alignment = Alignment;

    typedef value_type *pointer;
    typedef const value_type *const_pointer;

    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;

    template<class Other>
    struct rebind
    {
        typedef AlignedAllocator<Other, Alignment> other;
    };

    pointer address(reference v) const noexcept
    {
        return (std::addressof(v));
    }

    const_pointer address(const_reference v) const noexcept
    {
        return (std::addressof(v));
    }

    AlignedAllocator() noexcept
    {
    }

    AlignedAllocator(const AlignedAllocator<T, Alignment>&) noexcept
    {
    }

    template<class Other>
    AlignedAllocator(const AlignedAllocator<Other, Alignment>&) noexcept
    {
    }

    template<class Other>
    AlignedAllocator<T, Alignment>& operator=(const AlignedAllocator<Other, Alignment>&)
    {
        return (*this);
    }

    void deallocate(pointer addr, size_type size)
    {
        AlignedFree(addr);
    }

    pointer allocate(size_type size)
    {
        return static_cast<pointer>(AlignedMalloc(size, alignment));
    }

    pointer allocate(size_type size, const void *)
    {
        return (allocate(size));
    }

    template<class U, class... Types>
    void construct(U *addr, Types&&... args)
    {
        ::new ((void *)addr) U(std::forward<Types>(args)...);
    }


    template<class U>
    void destroy(U *obj)
    {
        obj->~U();
    }

    size_t max_size() const noexcept
    {
        return ((size_t)(-1) / sizeof(T));
    }
};

static inline void InvertX(float3 *dst, size_t num)
{
#ifdef usdiEnableISPC
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
#endif
}

static inline void Scale(float3 *dst, float s, size_t num)
{
#ifdef usdiEnableISPC
    ispc::ScaleF((float*)dst, s, (int)num*3);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
#endif
}

} // namespace usdi
