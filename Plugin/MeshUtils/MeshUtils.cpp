#include "pch.h"
#include "MeshUtils.h"
#include "mikktspace.h"

namespace mu {

void InvertX_Generic(float3 *dst, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
void InvertX_Generic(float4 *dst, size_t num)
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


struct TSpaceContext
{
    float4 *dst;
    const float3 *p;
    const float3 *n;
    const float2 *t;
    const int *counts;
    const int *offsets;
    const int *indices;
    size_t num_points;
    size_t num_faces;

    static int getNumFaces(const SMikkTSpaceContext *tctx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->num_faces;
    }

    static int getCount(const SMikkTSpaceContext *tctx, int i)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        return (int)_this->counts[i];
    }

    static void getPosition(const SMikkTSpaceContext *tctx, float *o_pos, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_pos = _this->p[face[ivtx]];
    }

    static void getNormal(const SMikkTSpaceContext *tctx, float *o_normal, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float3&)*o_normal = _this->n[face[ivtx]];
    }

    static void getTexCoord(const SMikkTSpaceContext *tctx, float *o_tcoord, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        (float2&)*o_tcoord = _this->t[face[ivtx]];
    }

    static void setTangent(const SMikkTSpaceContext *tctx, const float* tangent, const float* /*bitangent*/,
        float /*fMagS*/, float /*fMagT*/, tbool IsOrientationPreserving, int iface, int ivtx)
    {
        auto *_this = reinterpret_cast<TSpaceContext*>(tctx->m_pUserData);
        const int *face = &_this->indices[_this->offsets[iface]];
        float sign = (IsOrientationPreserving != 0) ? 1.0f : -1.0f;
        _this->dst[face[ivtx]] = { tangent[0], tangent[1], tangent[2], sign };
    }
};

bool CalculateTangents(
    float4 *dst, const float3 *p, const float3 *n, const float2 *t,
    const int *counts, const int *offsets, const int *indices, size_t num_points, size_t num_faces)
{
    TSpaceContext ctx = {dst, p, n, t, counts, offsets, indices, num_points, num_faces};

    SMikkTSpaceInterface iface;
    memset(&iface, 0, sizeof(iface));
    iface.m_getNumFaces = TSpaceContext::getNumFaces;
    iface.m_getNumVerticesOfFace = TSpaceContext::getCount;
    iface.m_getPosition = TSpaceContext::getPosition;
    iface.m_getNormal   = TSpaceContext::getNormal;
    iface.m_getTexCoord = TSpaceContext::getTexCoord;
    iface.m_setTSpace   = TSpaceContext::setTangent;

    SMikkTSpaceContext tctx;
    memset(&tctx, 0, sizeof(tctx));
    tctx.m_pInterface = &iface;
    tctx.m_pUserData = &ctx;

    return genTangSpaceDefault(&tctx) != 0;
}

template<class VertexT> static inline void InterleaveImpl(VertexT *dst, const typename VertexT::source_t& src, size_t i);

template<> inline void InterleaveImpl(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
}
template<> inline void InterleaveImpl(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
}
template<> inline void InterleaveImpl(vertex_v3n3u2t4 *dst, const vertex_v3n3u2t4::source_t& src, size_t i)
{
    dst[i].p = src.points[i];
    dst[i].n = src.normals[i];
    dst[i].u = src.uvs[i];
    dst[i].t = src.tangents[i];
}

template<class VertexT>
void Interleave_Generic(VertexT *dst, const typename VertexT::source_t& src, size_t num)
{
    for (size_t i = 0; i < num; ++i) {
        InterleaveImpl(dst, src, i);
    }
}

#ifdef muEnableISPC
#include "MeshUtilsCore.h"

void InvertX_ISPC(float3 *dst, size_t num)
{
    ispc::InvertXF3((ispc::float3*)dst, (int)num);
}
void InvertX_ISPC(float4 *dst, size_t num)
{
    ispc::InvertXF4((ispc::float4*)dst, (int)num);
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
#endif



#ifdef muEnableISPC
    #define Forward(Name, ...) Name##_ISPC(__VA_ARGS__)
#else
    #define Forward(Name, ...) Name##_Generic(__VA_ARGS__)
#endif

void InvertX(float3 *dst, size_t num)
{
    Forward(InvertX, dst, num);
}
void InvertX(float4 *dst, size_t num)
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
    Interleave_Generic(dst, src, num);
}
template void Interleave(vertex_v3n3 *dst, const vertex_v3n3::source_t& src, size_t num);
template void Interleave(vertex_v3n3u2 *dst, const vertex_v3n3u2::source_t& src, size_t num);
template void Interleave(vertex_v3n3u2t4 *dst, const vertex_v3n3u2t4::source_t& src, size_t num);


} // namespace mu
