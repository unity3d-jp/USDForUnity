#include "pch.h"
#include "aiMath.h"
#include "RawVector.h"


// ispc implementation
#ifdef aiEnableISPC
#include "aiSIMD.h"

void ApplyScaleISPC(float *dst, int num, float scale)
{
    ispc::Scale(dst, num, scale);
}
void ApplyScaleISPC(float3 *dst, int num, float scale)
{
    ispc::Scale((float*)dst, num * 3, scale);
}

void NormalizeISPC(float3 *dst, int num)
{
    ispc::Normalize((ispc::float3*)dst, num);
}

void LerpISPC(float2 *dst, const float2 * v1, const float2 * v2, int num, float w)
{
    ispc::Lerp((float*)dst, (float*)v1, (float*)v2, num * 2, w);
}
void LerpISPC(float3 *dst, const float3 * v1, const float3 * v2, int num, float w)
{
    ispc::Lerp((float*)dst, (float*)v1, (float*)v2, num * 3, w);
}
void LerpISPC(float4 *dst, const float4 * v1, const float4 * v2, int num, float w)
{
    ispc::Lerp((float*)dst, (float*)v1, (float*)v2, num * 4, w);
}

void GenerateVelocitiesISPC(float3 *dst, const float3 *p1, const float3 *p2, int num, float motion_scale)
{
    ispc::GenerateVelocities(
        (ispc::float3*)dst, (ispc::float3*)p1, (ispc::float3*)p2, num, motion_scale);
}

void MinMaxISPC(float3 & min, float3 & max, const float3 * points, int num)
{
    ispc::MinMax3((ispc::float3&)min, (ispc::float3&)max, (const ispc::float3*)points, num);
}

void GenerateNormalsISPC(float3 * dst, const float3 * points, const int * indices, int num_points, int num_triangles)
{
    ispc::GenerateNormalsTriangleIndexed((ispc::float3*)dst, (const ispc::float3*)points, indices, num_points, num_triangles);
}

void GenerateTangentsISPC(float4 *dst,
    const float3 *points, const float2 *uv, const float3 *normals, const int *indices,
    int num_points, int num_triangles)
{
    ispc::GenerateTangentsTriangleIndexed(
        (ispc::float4*)dst, (const ispc::float3*)points, (const ispc::float2*)uv, (const ispc::float3*)normals, indices, num_points, num_triangles);
}

#endif // aiEnableISPC


// < generic implementation

void ApplyScaleGeneric(float *dst, int num, float scale)
{
    for (int i = 0; i < num; ++i) {
        dst[i] *= scale;
    }
}
void ApplyScaleGeneric(float3 *dst, int num, float scale)
{
    for (int i = 0; i < num; ++i) {
        dst[i] *= scale;
    }
}

void LerpGeneric(float2 *dst, const float2 *v1, const float2 *v2, int num, float w)
{
    float iw = 1.0f - w;
    for (int i = 0; i < num; ++i) {
        dst[i] = (v1[i] * iw) + (v2[i] * w);
    }
}
void LerpGeneric(float3 *dst, const float3 *v1, const float3 *v2, int num, float w)
{
    float iw = 1.0f - w;
    for (int i = 0; i < num; ++i) {
        dst[i] = (v1[i] * iw) + (v2[i] * w);
    }
}
void LerpGeneric(float4 *dst, const float4 *v1, const float4 *v2, int num, float w)
{
    float iw = 1.0f - w;
    for (int i = 0; i < num; ++i) {
        dst[i] = (v1[i] * iw) + (v2[i] * w);
    }
}

void NormalizeGeneric(float3 *dst_, int num)
{
    auto *dst = (float3*)dst_;
    for (int i = 0; i < num; ++i) {
        dst[i] = normalize(dst[i]);
    }
}

void GenerateVelocitiesGeneric(float3 *dst, const float3 *p1, const float3 *p2, int num, float motion_scale)
{
    for (int i = 0; i < num; ++i) {
        dst[i] = (p2[i] - p1[i]) * motion_scale;
    }
}

void MinMaxGeneric(float3 &dst_min, float3 &dst_max, const float3 *src_, int num)
{
    if (num == 0) { return; }

    auto *src = (const float3*)src_;
    auto rmin = src[0];
    auto rmax = src[0];
    for (int i = 1; i < num; ++i) {
        auto t = src[i];
        rmin = min(rmin, t);
        rmax = max(rmax, t);
    }
    dst_min = (float3&)rmin;
    dst_max = (float3&)rmax;
}

void GenerateNormalsGeneric(float3 *dst, const float3 *points, const int *indices, int num_points, int num_triangles)
{
    memset(dst, 0, sizeof(float3)*num_points);
    for (int ti = 0; ti < num_triangles; ++ti) {
        int ti3 = ti * 3;
        auto p0 = points[indices[ti3 + 0]];
        auto p1 = points[indices[ti3 + 1]];
        auto p2 = points[indices[ti3 + 2]];
        auto n = cross(p1 - p0, p2 - p0);

        for (int i = 0; i < 3; ++i) {
            dst[indices[ti3 + i]] += n;
        }
    }
    for (int vi = 0; vi < num_points; ++vi) {
        dst[vi] = normalize(dst[vi]);
    }
}


void GenerateTangentsGeneric(float4 *dst_,
    const float3 *points_, const float2 *uv_, const float3 *normals_, const int *indices,
    int num_points, int num_triangles)
{
    auto *dst = (float4*)dst_;
    auto *points = (const float3*)points_;
    auto *uv = (const float2*)uv_;
    auto *normals = (const float3*)normals_;

    RawVector<float3> tangents, binormals;
    tangents.resize_zeroclear(num_points);
    binormals.resize_zeroclear(num_points);

    for (int ti = 0; ti < num_triangles; ++ti) {
        int ti3 = ti * 3;
        int idx[3] = { indices[ti3 + 0], indices[ti3 + 1], indices[ti3 + 2] };
        float3 v[3] = { points[idx[0]], points[idx[1]], points[idx[2]] };
        float2 u[3] = { uv[idx[0]], uv[idx[1]], uv[idx[2]] };
        float3 t[3];
        float3 b[3];
        compute_triangle_tangent(v, u, t, b);

        for (int i = 0; i < 3; ++i) {
            tangents[idx[i]] += t[i];
            binormals[idx[i]] += b[i];
        }
    }

    for (int vi = 0; vi < num_points; ++vi) {
        dst[vi] = orthogonalize_tangent(tangents[vi], binormals[vi], normals[vi]);
    }
}


// > generic implementation


// SIMD can't make this faster
void SwapHandedness(float3 *dst, int num)
{
    for (int i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}
void SwapHandedness(float4 *dst, int num)
{
    for (int i = 0; i < num; ++i) {
        dst[i].x *= -1.0f;
    }
}

#ifdef aiEnableISPC
    #define Impl(Func, ...) Func##ISPC(__VA_ARGS__)
#else
    #define Impl(Func, ...) Func##Generic(__VA_ARGS__)
#endif

void ApplyScale(float *dst, int num, float scale)
{
    Impl(ApplyScale, dst, num, scale);
}
void ApplyScale(float3 *dst, int num, float scale)
{
    Impl(ApplyScale, dst, num, scale);
}

void Normalize(float3 *dst, int num)
{
    Impl(Normalize, dst, num);
}

void Lerp(float2 *dst, const float2 *v1, const float2 *v2, int num, float w)
{
    Impl(Lerp, dst, v1, v2, num, w);
}
void Lerp(float3 *dst, const float3 *v1, const float3 *v2, int num, float w)
{
    Impl(Lerp, dst, v1, v2, num, w);
}
void Lerp(float4 *dst, const float4 *v1, const float4 *v2, int num, float w)
{
    Impl(Lerp, dst, v1, v2, num, w);
}


void GenerateVelocities(float3 *dst, const float3 *p1, const float3 *p2, int num, float motion_scale)
{
    Impl(GenerateVelocities, dst, p1, p2, num, motion_scale);
}


void MinMax(float3 &min, float3 &max, const float3 *points, int num)
{
    Impl(MinMax, min, max, points, num);
}

void GenerateNormals(float3 *dst, const float3 *points, const int *indices, int num_points, int num_triangles)
{
    Impl(GenerateNormals, dst, points, indices, num_points, num_triangles);
}

void GenerateTangents(float4 *dst,
    const float3 *points, const float2 *uv, const float3 *normals, const int *indices,
    int num_points, int num_triangles)
{
    Impl(GenerateTangents, dst, points, uv, normals, indices, num_points, num_triangles);
}

#undef Impl


