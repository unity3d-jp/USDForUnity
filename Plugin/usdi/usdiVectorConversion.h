#pragma once

#include "usdiInternal.h"

namespace usdi {

template<class Dst, class Src> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src); }
// force make convertible GfVec2[hfd] <-> GfVec3[hfd] <-> GfVec4[hfd]
#define V2(Dst, Src) template<> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src[0], src[1]); }
V2(GfVec2h, GfVec3h) V2(GfVec2h, GfVec3f) V2(GfVec2h, GfVec3d) V2(GfVec2h, GfVec4h) V2(GfVec2h, GfVec4f) V2(GfVec2h, GfVec4d)
V2(GfVec2f, GfVec3h) V2(GfVec2f, GfVec3f) V2(GfVec2f, GfVec3d) V2(GfVec2f, GfVec4h) V2(GfVec2f, GfVec4f) V2(GfVec2f, GfVec4d)
V2(GfVec2d, GfVec3h) V2(GfVec2d, GfVec3f) V2(GfVec2d, GfVec3d) V2(GfVec2d, GfVec4h) V2(GfVec2d, GfVec4f) V2(GfVec2d, GfVec4d)
#undef V2
#define V3(Dst, Src) template<> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src[0], src[1], 0); }
V3(GfVec3h, GfVec2h) V3(GfVec3h, GfVec2f) V3(GfVec3h, GfVec2d)
V3(GfVec3f, GfVec2h) V3(GfVec3f, GfVec2f) V3(GfVec3f, GfVec2d)
V3(GfVec3d, GfVec2h) V3(GfVec3d, GfVec2f) V3(GfVec3d, GfVec2d)
#undef V3
#define V3(Dst, Src) template<> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src[0], src[1], src[2]); }
V3(GfVec3h, GfVec4h) V3(GfVec3h, GfVec4f) V3(GfVec3h, GfVec4d)
V3(GfVec3f, GfVec4h) V3(GfVec3f, GfVec4f) V3(GfVec3f, GfVec4d)
V3(GfVec3d, GfVec4h) V3(GfVec3d, GfVec4f) V3(GfVec3d, GfVec4d)
#undef V3
#define V4(Dst, Src) template<> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src[0], src[1], 0, 0); }
V4(GfVec4h, GfVec2h) V4(GfVec4h, GfVec2f) V4(GfVec4h, GfVec2d)
V4(GfVec4f, GfVec2h) V4(GfVec4f, GfVec2f) V4(GfVec4f, GfVec2d)
V4(GfVec4d, GfVec2h) V4(GfVec4d, GfVec2f) V4(GfVec4d, GfVec2d)
#undef V4
#define V4(Dst, Src) template<> inline void VAssign(Dst& dst, const Src& src) { dst = Dst(src[0], src[1], src[2], 0); }
V4(GfVec4h, GfVec3h) V4(GfVec4h, GfVec3f) V4(GfVec4h, GfVec3d)
V4(GfVec4f, GfVec3h) V4(GfVec4f, GfVec3f) V4(GfVec4f, GfVec3d)
V4(GfVec4d, GfVec3h) V4(GfVec4d, GfVec3f) V4(GfVec4d, GfVec3d)
#undef V4


template<class T, class U>
struct VConvert
{
    void operator()(VtArray<T>& dst, const VtArray<U>& src)
    {
        size_t n = src.size();
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { VAssign(dst[i], src[i]); }
    }
    void operator()(T* dst, size_t dst_len, const VtArray<T>& src)
    {
        size_t n = std::min<size_t>(dst_len, src.size());
        for (size_t i = 0; i < n; ++i) { dst[i] = src[i]; }
    }
    void operator()(VtArray<U>& dst, const VtArray<T>& src)
    {
        size_t n = src.size();
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { VAssign(dst[i], src[i]); }
    }
    void operator()(VtArray<U>& dst, const T *src, size_t src_len)
    {
        size_t n = src_len;
        dst.resize(n);
        for (size_t i = 0; i < n; ++i) { VAssign(dst[i], src[i]); }
    }
};

#define Impl(V1, E1, V2, E2, L, V1ToV2, V2ToV1)\
template<>\
struct VConvert<V1, V2>\
{\
    void operator()(VtArray<V1>& dst, const VtArray<V2>& src)\
    {\
        size_t n = src.size();\
        dst.resize(n);\
        V2ToV1((E1*)dst.data(), (const E2*)src.cdata(), n*L);\
    }\
    void operator()(V1* dst, size_t dst_len, const VtArray<V1>& src)\
    {\
        size_t n = std::min<size_t>(dst_len, src.size());\
        for (size_t i = 0; i < n; ++i) { dst[i] = src[i]; }\
    }\
    void operator()(VtArray<V2>& dst, const VtArray<V1>& src)\
    {\
        size_t n = src.size();\
        dst.resize(n);\
        V1ToV2((E2*)dst.data(), (const E1*)src.cdata(), n*L);\
    }\
    void operator()(VtArray<V2>& dst, const V1 *src, size_t src_len)\
    {\
        size_t n = src_len;\
        dst.resize(n);\
        V1ToV2((E2*)dst.data(), (const E1*)src, n*L);\
    }\
};

//Impl(  float, float,    half,  half, 1, FloatToHalf, HalfToFloat)
//Impl(GfVec2f, float, GfVec2h,  half, 2, FloatToHalf, HalfToFloat)
//Impl(GfVec3f, float, GfVec3h,  half, 3, FloatToHalf, HalfToFloat)
//Impl(GfVec4f, float, GfVec4h,  half, 4, FloatToHalf, HalfToFloat)
//Impl(   half,  half,   float, float, 1, HalfToFloat, FloatToHalf)
//Impl(GfVec2h,  half, GfVec2f, float, 2, HalfToFloat, FloatToHalf)
//Impl(GfVec3h,  half, GfVec3f, float, 3, HalfToFloat, FloatToHalf)
//Impl(GfVec4h,  half, GfVec4f, float, 4, HalfToFloat, FloatToHalf)

#undef Impl


} // namespace usdi
