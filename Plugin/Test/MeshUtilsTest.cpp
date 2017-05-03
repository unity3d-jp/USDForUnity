#include <cstdio>
#include <vector>
#include <chrono>
#include "Mesh.h"
using namespace mu;

// force to align 0x20 for AVX functions
void* operator new(std::size_t s) throw(std::bad_alloc)
{
    return usdiAlignedMalloc(s, 0x20);
}
void operator delete(void *addr) throw()
{
    usdiAlignedFree(addr);
}

template<class T>
inline bool near_equal(const std::vector<T>& a, const std::vector<T>& b, float epsilon = muDefaultEpsilon)
{
    if (a.size() != b.size()) { return false; }

    for (size_t i = 0; i < a.size(); ++i) {
        if (!near_equal(a[i], b[i], epsilon)) {
            return false;
        }
    }
    return true;
}

template<class T, size_t S>
inline bool near_equal(const T (&a)[S], const T (&b)[S], float epsilon = muDefaultEpsilon)
{
    for (size_t i = 0; i < S; ++i) {
        if (!near_equal(a[i], b[i], epsilon)) {
            return false;
        }
    }
    return true;
}

typedef uint64_t ns;

static ns now()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}


static std::vector<float3> GenerateFloat3Array(size_t num, float cycle, float scale)
{
    std::vector<float3> ret(num);
    for (size_t i = 0; i < num; ++i) {
        ret[i] = {
            std::cos(cycle*i*0.3f) * scale * 0.5f,
            0.0f,
            std::sin(cycle*i*1.1f) * scale * 0.5f
        };
    }
    return ret;
}



#define NumTestData (1024*1024)
#define NumTry 8

static void Test_HalfConversion()
{
    std::vector<float> src(NumTestData);
    std::vector<half> dst_h1(NumTestData);
    std::vector<half> dst_h2(NumTestData);
    std::vector<float> dst_f1(NumTestData);
    std::vector<float> dst_f2(NumTestData);

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTestData; ++i) {
        src[i] = (float)i / NumTestData;
    }

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        FloatToHalf_Generic(dst_h1.data(), src.data(), src.size());
        HalfToFloat_Generic(dst_f1.data(), dst_h1.data(), src.size());
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        FloatToHalf_ISPC(dst_h2.data(), src.data(), src.size());
        HalfToFloat_ISPC(dst_f2.data(), dst_h2.data(), src.size());
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(dst_f1, dst_f2, 0.01f);
        if (!result) { break; }
    }

    printf("Test_HalfConversion: %s\n", result ? "succeeded" : "failed");
    printf("    FloatToHalf & HalfToFloat Generic: avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    FloatToHalf & HalfToFloat ISPC: avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}

static void Test_InvertX()
{
    auto data1 = GenerateFloat3Array(NumTestData, 0.1f, 1.0f);
    auto data2 = data1;

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        InvertX_Generic(data1.data(), data1.size());
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        InvertX_ISPC(data2.data(), data2.size());
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(data1, data2);
        if (!result) { break; }
    }

    printf("Test_InvertX: %s\n", result ? "succeeded" : "failed");
    printf("    InvertX_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    InvertX_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}


static void Test_Scale()
{
    auto data1 = GenerateFloat3Array(NumTestData, 0.1f, 1.0f);
    auto data2 = data1;
    auto scale = 12.345f;

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        Scale_Generic(data1.data(), scale, data1.size());
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        Scale_ISPC(data2.data(), scale, data2.size());
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(data1, data2);
        if (!result) { break; }
    }

    printf("Test_Scale: %s\n", result ? "succeeded" : "failed");
    printf("    Scale_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    Scale_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}


static void Test_MinMax()
{
    auto data = GenerateFloat3Array(NumTestData, 0.1f, 1.0f);
    float3 bounds1[2];
    float3 bounds2[2];

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        MinMax_Generic(data.data(), data.size(), bounds1[0], bounds1[1]);
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        MinMax_ISPC(data.data(), data.size(), bounds2[0], bounds2[1]);
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(bounds1, bounds2);
        if (!result) { break; }
    }

    printf("Test_MinMax: %s\n", result ? "succeeded" : "failed");
    printf("    MinMax_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    MinMax_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}


static void Test_Normalize()
{
    auto data1 = GenerateFloat3Array(NumTestData, 0.1f, 1.0f);
    auto data2 = data1;

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        Normalize_Generic(data1.data(), data1.size());
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        Normalize_ISPC(data2.data(), data2.size());
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(data1, data2);
        if (!result) { break; }
    }

    printf("Test_Normalize: %s\n", result ? "succeeded" : "failed");
    printf("    Normalize_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    Normalize_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}


void Test_Interleave()
{
    float3 point = { 0.0f, 1.0f, 2.0f };
    float3 normal = { 3.0f, 4.0f, 5.0f };
    float4 color = { 6.0f, 7.0f, 8.0f, 9.0f };
    float2 uv = { 10.0f, 11.0f };
    float4 tangent = { 12.0f, 13.0f, 14.0f, 15.0f };

    printf("Test_Interleave:\n");
    {
        vertex_v3n3 dst;
        auto format = GuessVertexFormat(&point, &normal, nullptr, nullptr, nullptr);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, nullptr, nullptr, nullptr);
        bool result =
            format == VertexFormat::V3N3 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal;
        printf("    vertex_v3n3: %s\n", result ? "succeeded" : "failed");
    }
    {
        vertex_v3n3c4 dst;
        auto format = GuessVertexFormat(&point, &normal, &color, nullptr, nullptr);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, &color, nullptr, nullptr);
        bool result =
            format == VertexFormat::V3N3C4 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal && dst.c == color;
        printf("    vertex_v3n3c4: %s\n", result ? "succeeded" : "failed");
    }
    {
        vertex_v3n3u2 dst;
        auto format = GuessVertexFormat(&point, &normal, nullptr, &uv, nullptr);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, nullptr, &uv, nullptr);
        bool result =
            format == VertexFormat::V3N3U2 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal && dst.u == uv;
        printf("    vertex_v3n3u2: %s\n", result ? "succeeded" : "failed");
    }
    {
        vertex_v3n3c4u2 dst;
        auto format = GuessVertexFormat(&point, &normal, &color, &uv, nullptr);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, &color, &uv, nullptr);
        bool result =
            format == VertexFormat::V3N3C4U2 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal&& dst.c == color && dst.u == uv;
        printf("    vertex_v3n3c4u2: %s\n", result ? "succeeded" : "failed");
    }
    {
        vertex_v3n3u2t4 dst;
        auto format = GuessVertexFormat(&point, &normal, nullptr, &uv, &tangent);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, nullptr, &uv, &tangent);
        bool result =
            format == VertexFormat::V3N3U2T4 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal && dst.u == uv && dst.t == tangent;
        printf("    vertex_v3n3u2t4: %s\n", result ? "succeeded" : "failed");
    }
    {
        vertex_v3n3c4u2t4 dst;
        auto format = GuessVertexFormat(&point, &normal, &color, &uv, &tangent);
        auto size = GetVertexSize(format);
        Interleave(&dst, format, 1, &point, &normal, &color, &uv, &tangent);
        bool result =
            format == VertexFormat::V3N3C4U2T4 && size == sizeof(dst) &&
            dst.p == point && dst.n == normal && dst.u == uv && dst.t == tangent;
        printf("    vertex_v3n3c4u2t4: %s\n", result ? "succeeded" : "failed");
    }
}

void MeshUtilsTest()
{
    Test_HalfConversion();
    Test_InvertX();
    Test_Scale();
    Test_MinMax();
    Test_Normalize();
    Test_Interleave();
}
