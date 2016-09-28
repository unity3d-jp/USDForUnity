#pragma once

#include "vector.h"

namespace mu {


void InvertX(float3 *dst, size_t num);
void Scale(float3 *dst, float s, size_t num);
void ComputeBounds(const float3 *p, size_t num, float3& o_min, float3& o_max);
void Normalize(float3 *dst, size_t num);
void CalculateNormals(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices);

struct vertex_v3n3;
struct vertex_v3n3_source;
struct vertex_v3n3u2;
struct vertex_v3n3u2_source;

struct vertex_v3n3_source
{
    typedef vertex_v3n3 vertex_t;
    const float3 *points;
    const float3 *normals;
};

struct vertex_v3n3
{
    typedef vertex_v3n3_source source_t;
    float3 p;
    float3 n;
};

struct vertex_v3n3u2_source
{
    typedef vertex_v3n3u2 vertex_t;
    const float3 *points;
    const float3 *normals;
    const float2 *uvs;
};

struct vertex_v3n3u2
{
    typedef vertex_v3n3u2_source source_t;
    float3 p;
    float3 n;
    float2 u;
};

template<class VertexT>
void Interleave(VertexT *dst, const typename VertexT::source_t& src, size_t num);

template<class DataArray, class IndexArray>
void CopyWithIndices(DataArray& dst, const DataArray& src, const IndexArray& indices, size_t beg, size_t end, bool expand);



// ------------------------------------------------------------
// internal
// ------------------------------------------------------------
void InvertX_Generic(float3 *dst, size_t num);
void InvertX_ISPC(float3 *dst, size_t num);

void Scale_Generice(float3 *dst, float s, size_t num);
void Scale_ISPC(float3 *dst, float s, size_t num);

void ComputeBounds_Generic(const float3 *p, size_t num, float3& o_min, float3& o_max);
void ComputeBounds_ISPC(const float3 *p, size_t num, float3& o_min, float3& o_max);

void Normalize_Generic(float3 *dst, size_t num);
void Normalize_ISPC(float3 *dst, size_t num);

void CalculateNormals_Generic(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices);
void CalculateNormals_ISPC(float3 *dst, const float3 *p, const int *indices, size_t num_points, size_t num_indices);

template<class VertexT> void Interleave_Generic(VertexT *dst, const typename VertexT::source_t& src, size_t num);
template<class VertexT> void Interleave_ISPC(VertexT *dst, const typename VertexT::source_t& src, size_t num);

// ------------------------------------------------------------
// impl
// ------------------------------------------------------------

template<class DataArray, class IndexArray>
inline void CopyWithIndices(DataArray& dst, const DataArray& src, const IndexArray& indices, size_t beg, size_t end, bool expand)
{
    if (src.empty()) { return; }

    size_t size = end - beg;
    dst.resize(size);

    if (expand) {
        for (int i = 0; i < size; ++i) {
            dst[i] = src[indices[beg + i]];
        }
    }
    else {
        for (int i = 0; i < size; ++i) {
            dst[i] = src[beg + i];
        }
    }
}

} // namespace mu
