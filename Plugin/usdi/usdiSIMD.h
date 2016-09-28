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


class TempBuffer
{
public:
    TempBuffer() {}
    TempBuffer(const TempBuffer& v) = delete;
    TempBuffer& operator=(const TempBuffer& v) = delete;

    ~TempBuffer()
    {
        clear();
    }

    size_t size() const { return m_size; }
    void* data() { return m_data; }
    const void* data() const { return m_data; }
    const void* cdata() const { return m_data; }

    void resize(size_t s)
    {
        if (s > m_capacity) {
            clear();
            m_data = AlignedMalloc(s, 0x20);
            m_capacity = s;
        }
        m_size = s;
    }

    void clear()
    {
        AlignedFree(m_data);
        m_data = nullptr;
        m_size = m_capacity = 0;
    }

private:
    void *m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
};

TempBuffer& GetTemporaryBuffer();
void InvertX(float3 *dst, size_t num);
void Scale(float3 *dst, float s, size_t num);
void ComputeBounds(const float3 *p, size_t num, float3& o_min, float3& o_max);


struct vertex_v3n3
{
    float3 p;
    float3 n;
};

struct vertex_v3n3u2
{
    float3 p;
    float3 n;
    float2 u;
};

template<class VertexT> void WriteVertices(VertexT *dst, const MeshData& src);

template<class VertexT>
inline void WriteVertices(TempBuffer& buf, const MeshData& src)
{
    using vertex_t = VertexT;
    buf.resize(sizeof(vertex_t) * src.num_points);
    WriteVertices((VertexT*)buf.data(), src);
}

} // namespace usdi
