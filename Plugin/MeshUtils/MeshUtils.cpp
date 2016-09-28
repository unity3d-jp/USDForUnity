#include "pch.h"
#include "MeshUtils.h"
#include "MeshUtilsCore.h"

namespace mu {

void InvertX_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}

void Scale_Generic(float3 *dst, float s, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] *= s;
    }
}

void ComputeBounds_Generic(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
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
    omin = rmin;
    omax = rmax;
}

void Normalize_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

void CalculateNormals_Generic(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t i = 0; i < num_indices; i += 3)
    {
        int i0 = indices[i + 0];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];
        float3 p0 = p[i0];
        float3 p1 = p[i1];
        float3 p2 = p[i2];
        float3 n = cross(p1 - p0, p2 - p0);
        dst[i0] += n;
        dst[i1] += n;
        dst[i2] += n;
    }

    for (size_t i = 0; i < num_points; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

template<class VertexT> static inline void InterleaveImpl(VertexT *dst, const typename VertexT::source_t& src, size_t i);

template<> static inline void InterleaveImpl(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}

template<> static inline void InterleaveImpl(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}

template<class VertexT>
void Interleave_Generic(VertexT *dst, const typename VertexT::source_t& src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        InterleaveImpl(dst, src, i);
    }
}

#ifdef muUseISPC
void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
}

void Scale_ISPC(float3 *dst, float s, size_t num)
{
    ispc::ScaleF((float*)dst, s, (int)num * 3);
}

void ComputeBounds_ISPC(const float3 *p, size_t num, float3& omin, float3& omax)
{
    if (num == 0) { return; }
    ispc::ComputeBounds((ispc::float3*)p, (int)num, (ispc::float3&)omin, (ispc::float3&)omax);
}

void Normalize_ISPC(float3 *dst, size_t num)
{
    ispc::Normalize((ispc::float3*)dst, (int)num);
}

void CalculateNormals_ISPC(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices)
{
    memset(dst, 0, sizeof(float3)*num_points);

    for (size_t i = 0; i < num_indices; i += 3)
    {
        int i0 = indices[i + 0];
        int i1 = indices[i + 1];
        int i2 = indices[i + 2];
        float3 p0 = p[i0];
        float3 p1 = p[i1];
        float3 p2 = p[i2];
        float3 n = cross(p1 - p0, p2 - p0);
        dst[i0] += n;
        dst[i1] += n;
        dst[i2] += n;
    }

    ispc::Normalize((ispc::float3*)dst, (int)num_points);
}

template<>
void Interleave_ISPC(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t num)
{
    ispc::InterleaveV3N3(
        (ispc::vertex_v3n3*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (int)num);
}
template<>
void Interleave_ISPC(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t num)
{
    ispc::InterleaveV3N3U2(
        (ispc::vertex_v3n3u2*)dst,
        (ispc::float3*)src.points,
        (ispc::float3*)src.normals,
        (ispc::float2*)src.uvs,
        (int)num);
}
#endif





#ifdef muUseISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}

void Scale(float3 *dst, float s, size_t num)
{
    Forward(Scale, dst, s, num);
}

void ComputeBounds(const float3 *p, size_t num, float3& omin, float3& omax)
{
    Forward(ComputeBounds, p, num, omin, omax);
}

void Normalize(float3 *dst, size_t num)
{
    Forward(Normalize, dst, num);
}

void CalculateNormals(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices)
{
    Forward(CalculateNormals, dst, p, indices, num_points, num_indices);
}

template<class VertexT>
void Interleave(VertexT *dst, const typename VertexT::source_t& src, size_t num)
{
    Forward(Interleave, dst, src, num);
}
template void Interleave(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t num);
template void Interleave(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t num);


} // namespace mu
