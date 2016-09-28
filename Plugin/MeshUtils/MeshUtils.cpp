#include "pch.h"
#include "MeshUtils.h"
#include "MeshUtilsCore.h"

namespace mu {

void InvertX(float3 *dst, size_t num)
{
#ifdef muUseISPC
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
#else
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
#endif
}

void Scale(float3 *dst, float s, size_t num)
{
#ifdef muUseISPC
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

#ifdef muUseISPC
    ispc::ComputeBounds((ispc::float3*)p, (int)num, (ispc::float3&)o_min, (ispc::float3&)o_max);
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

void CalculateNormals(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t i = 0; i < num_indices; i += 3)
    {
        int i0 = indices[i + 0];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];
        float3 n = cross(p[i1] - p[i0], p[i2] - p[i0]);
        dst[i0] += n;
        dst[i1] += n;
        dst[i2] += n;
    }

    for (size_t i = 0; i < num_points; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

#ifdef muUseISPC

template<>
void Interleave(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t num)
{
    ispc::InterleaveV3N3(
        (ispc::vertex_v3n3*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (int)num);
}

template<>
void Interleave(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t num)
{
    ispc::InterleaveV3N3U2(
        (ispc::vertex_v3n3u2*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (ispc::float2*)src.uvs,
        (int)num);
}

#else

template<class VertexT> static inline void InterleaveNth(VertexT *dst, const typename VertexT::source_t& src, size_t i);

template<> static inline void InterleaveNth(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}

template<> static inline void InterleaveNth(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}

template<class VertexT>
void Interleave(VertexT *dst, const typename VertexT::source_t& src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        InterleaveNth(dst, src, i);
    }
}

template void Interleave(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t num);
template void Interleave(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t num);
#endif

} // namespace mu
