#include <cstdio>
#include <vector>
#include <chrono>
#include "MeshUtils/MeshUtils.h"
using namespace mu;


void* operator new(std::size_t s) throw(std::bad_alloc)
{
    return AlignedMalloc(s, 0x20);
}

void operator delete(void *addr) throw()
{
    AlignedFree(addr);
}

template<class T>
inline bool near_equal(const std::vector<T>& a, const std::vector<T>& b)
{
    if (a.size() != b.size()) { return false; }

    for (size_t i = 0; i < a.size(); ++i) {
        if (!near_equal(a[i], b[i])) { return false; }
    }
    return true;
}

template<class T, size_t S>
inline bool near_equal(const T (&a)[S], const T (&b)[S])
{
    for (size_t i = 0; i < S; ++i) {
        if (!near_equal(a[i], b[i])) { return false; }
    }
    return true;
}


std::vector<float3> GenerateTestData(size_t num, float cycle, float scale)
{
    std::vector<float3> ret;
    for (size_t i = 0; i < num; ++i) {
        ret.push_back({ std::cos(cycle*i*0.3f)*scale, std::sin(cycle*i*0.7f)*scale, std::sin(cycle*i*1.1f)*scale});
    }
    return ret;
}


void Test_InvertX()
{
    auto data1 = GenerateTestData(1024 * 1024, 0.1f, 1.0f);
    auto data2 = data1;

    InvertX_Generic(data1.data(), data1.size());
    InvertX_ISPC(data2.data(), data2.size());
    auto result = near_equal(data1, data2);

    printf("Test_InvertX: %s\n", result ? "succeeded" : "failed");
}


void Test_Scale()
{
    auto data1 = GenerateTestData(1024 * 1024, 0.1f, 1.0f);
    auto data2 = data1;
    auto scale = 12.345f;

    Scale_Generic(data1.data(), scale, data1.size());
    Scale_ISPC(data2.data(), scale, data2.size());
    auto result = near_equal(data1, data2);

    printf("Test_Scale: %s\n", result ? "succeeded" : "failed");
}


void Test_ComputeBounds()
{
    auto data = GenerateTestData(1024 * 1024, 0.1f, 1.0f);
    float3 bounds1[2];
    float3 bounds2[2];

    ComputeBounds_Generic(data.data(), data.size(), bounds1[0], bounds1[1]);
    ComputeBounds_ISPC(data.data(), data.size(), bounds2[0], bounds2[1]);
    auto result = near_equal(bounds1, bounds2);

    printf("Test_ComputeBounds: %s\n", result ? "succeeded" : "failed");
}


void Test_Normalize()
{
    auto data1 = GenerateTestData(1024 * 1024, 0.1f, 1.0f);
    auto data2 = data1;

    Normalize_Generic(data1.data(), data1.size());
    Normalize_ISPC(data2.data(), data2.size());
    auto result = near_equal(data1, data2);

    printf("Test_Normalize: %s\n", result ? "succeeded" : "failed");
}


void Test_CalculateNormals()
{
    auto points = GenerateTestData(1024 * 1024, 0.1f, 1.0f);
    std::vector<int> indices;
    indices.resize(points.size());
    for (size_t i = 0; i < indices.size(); ++i) { indices[i] = (int)i; }

    std::vector<float3> normals1; normals1.resize(points.size());
    std::vector<float3> normals2; normals2.resize(points.size());

    CalculateNormals_Generic(normals1.data(), points.data(), indices.data(), points.size(), indices.size());
    CalculateNormals_ISPC(normals2.data(), points.data(), indices.data(), points.size(), indices.size());
    auto result = near_equal(normals1, normals2);

    printf("Test_CalculateNormals: %s\n", result ? "succeeded" : "failed");
}


int main(int argc, char *argv[])
{
    Test_InvertX();
    Test_Scale();
    Test_ComputeBounds();
    Test_Normalize();
    Test_CalculateNormals();
}
