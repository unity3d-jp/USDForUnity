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



TempBuffer& GetTemporaryBuffer()
{
    static thread_local TempBuffer s_buf;
    return s_buf;
}

void InvertX(float3 *dst, size_t num)
{
#ifdef usdiEnableISPC
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
#endif
}

void Scale(float3 *dst, float s, size_t num)
{
#ifdef usdiEnableISPC
    ispc::ScaleF((float*)dst, s, (int)num * 3);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
#endif
}

void ComputeBounds(const float3 *p, size_t num, float3& o_min, float3& o_max)
{
    if (num == 0) { return; }

#ifdef usdiEnableISPC
    ispc::ComputeBounds((ispc::float3*)p, num, (ispc::float3&)o_min, (ispc::float3&)o_max);
#else
    float3 rmin = p[0], rmax = p[0];
    for (size_t i = 1; i < num; ++i) {
        auto _ = p[i];
        rmin.x = std::min<float>(rmin.x, _.x);
        rmin.y = std::min<float>(rmin.y, _.y);
        rmin.z = std::min<float>(rmin.z, _.z);
        rmax.x = std::max<float>(rmax.x, _.x);
        rmax.y = std::max<float>(rmax.y, _.y);
        rmax.z = std::max<float>(rmax.z, _.z);
    }
    o_min = rmin;
    o_max = rmax;
#endif
}

#ifdef usdiEnableISPC

template<>
void WriteVertices(vertex_v3n3 *dst, const MeshData& src)
{
    ispc::InterleaveVerticesV3N3(
        (ispc::vertex_v3n3*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        src.num_points);
}

template<>
void WriteVertices(vertex_v3n3u2 *dst, const MeshData& src)
{
    ispc::InterleaveVerticesV3N3U2(
        (ispc::vertex_v3n3u2*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (ispc::float2*)src.uvs,
        src.num_points);
}

#else

template<class VertexT> static inline void WriteVertex(VertexT *dst, const MeshData& src, int i);

template<> static inline void WriteVertex(vertex_v3n3 *dst, const MeshData& src, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}

template<> static inline void WriteVertex(vertex_v3n3u2 *dst, const MeshData& src, int i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}

template<class VertexT>
void WriteVertices(VertexT *dst, const MeshData& src)
{
    for (int i = 0; i < src.num_points; ++i) {
        WriteVertex(dst, src, i);
    }
}

template void WriteVertices(vertex_v3n3 *dst, const MeshData& src);
template void WriteVertices(vertex_v3n3u2 *dst, const MeshData& src);
#endif

} // namespace usdi
