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
inline bool near_equal(const std::vector<T>& a, const std::vector<T>& b)
{
    if (a.size() != b.size()) { return false; }

    for (size_t i = 0; i < a.size(); ++i) {
        if (!near_equal(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

template<class T, size_t S>
inline bool near_equal(const T (&a)[S], const T (&b)[S])
{
    for (size_t i = 0; i < S; ++i) {
        if (!near_equal(a[i], b[i])) {
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
#define NumTry 16

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


static void Test_ComputeBounds()
{
    auto data = GenerateFloat3Array(NumTestData, 0.1f, 1.0f);
    float3 bounds1[2];
    float3 bounds2[2];

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        ComputeBounds_Generic(data.data(), data.size(), bounds1[0], bounds1[1]);
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        ComputeBounds_ISPC(data.data(), data.size(), bounds2[0], bounds2[1]);
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(bounds1, bounds2);
        if (!result) { break; }
    }

    printf("Test_ComputeBounds: %s\n", result ? "succeeded" : "failed");
    printf("    ComputeBounds_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    ComputeBounds_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
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


static void Test_GenerateNormals()
{
    std::vector<int> counts;
    std::vector<int> offsets;
    std::vector<int> indices;
    std::vector<float3> points;
    std::vector<float2> uv;
    std::vector<float3> normals;
    GenerateWaveMesh(counts, indices, points, uv, 1.0f, 0.5f, 1024, 0.0);
    GenerateOffsets(offsets, counts);

    ns elapsed1 = 0;
    ns elapsed2 = 0;
    bool result = false;

    std::vector<float3> normals1; normals1.resize(points.size());
    std::vector<float3> normals2; normals2.resize(points.size());

    for (int i = 0; i < NumTry; ++i) {
        auto start = now();
        GenerateNormals_Generic(normals1.data(), points.data(), counts.data(), offsets.data(), indices.data(), points.size(), counts.size());
        elapsed1 += now() - start;

#ifdef muEnableISPC
        start = now();
        GenerateNormals_ISPC(normals2.data(), points.data(), counts.data(), offsets.data(), indices.data(), points.size(), counts.size());
        elapsed2 += now() - start;
#endif // muEnableISPC

        result = near_equal(normals1, normals2);
        if (!result) { break; }
    }

    printf("Test_GenerateNormals: %s\n", result ? "succeeded" : "failed");
    printf("    CalculateNormals_Generic(): avg. %f ms\n", float(elapsed1 / NumTry) / 1000000.0f);
    printf("    CalculateNormals_ISPC(): avg. %f ms\n", float(elapsed2 / NumTry) / 1000000.0f);
    printf("\n");
}

void MeshUtilsTest()
{
    Test_InvertX();
    Test_Scale();
    Test_ComputeBounds();
    Test_Normalize();
    Test_GenerateNormals();
}
