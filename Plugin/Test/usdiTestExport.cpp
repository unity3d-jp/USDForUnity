#include <type_traits>
#include <cstdio>
#include <cmath>
#include "../usdi/usdi.h"

using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;


template<class T>
static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const T& v)
{
    usdi::Time t = 0.0;
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = (void*)&v;
    data.num_elements = 1;
    for (int i = 0; i < 5; ++i) {
        t += 1.0 / 30.0;
        usdiAttrWriteSample(attr, &data, t);
    }
}
template<class T, size_t N>
static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const T (&v)[N])
{
    usdi::Time t = 0.0;
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = (void*)v;
    data.num_elements = N;
    for (int i = 0; i < 5; ++i) {
        t += 1.0 / 30.0;
        usdiAttrWriteSample(attr, &data, t);
    }
}

static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const void *&v)
{
    usdi::Time t = 0.0;
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = (void*)v;
    data.num_elements = 1;
    for (int i = 0; i < 5; ++i) {
        t += 1.0 / 30.0;
        usdiAttrWriteSample(attr, &data, t);
    }
}


static void TestAttributes(usdi::Schema *schema)
{
    {
        int v = 123;
        AddAttribute(schema, "int_scalar", usdi::AttributeType::Int, v);
    }
    {
        float v = 1.23f;
        AddAttribute(schema, "float_scalar", usdi::AttributeType::Float, v);
    }
    {
        float2 v = {1.23f, 2.34f};
        AddAttribute(schema, "float2_scalar", usdi::AttributeType::Float2, v);
    }
    {
        float3 v = { 1.23f, 2.34f, 3.45f };
        AddAttribute(schema, "float3_scalar", usdi::AttributeType::Float3, v);
    }
    {
        float4 v = { 1.23f, 2.34f, 3.45f, 4.56f };
        AddAttribute(schema, "float4_scalar", usdi::AttributeType::Float4, v);
    }
    {
        quatf v = { 1.23f, 2.34f, 3.45f, 4.56f };
        AddAttribute(schema, "quaternion_scalar", usdi::AttributeType::Quaternion, v);
    }
    {
        const char *v = "test_token";
        AddAttribute(schema, "token_scalar", usdi::AttributeType::Token, v);
    }
    {
        const char *v = "test_string";
        AddAttribute(schema, "string_scalar", usdi::AttributeType::String, v);
    }

    {
        int v[] = { 123, 234, 345 };
        AddAttribute(schema, "int_array", usdi::AttributeType::IntArray, v);
    }
    {
        float v[] = { 1.23f, 2.34f, 3.45f };
        AddAttribute(schema, "float_array", usdi::AttributeType::FloatArray, v);
    }
    {
        float2 v[] = { { 1.23f, 2.34f } ,{ 3.45f, 4.56f } ,{ 5.67f, 6.78f } };
        AddAttribute(schema, "float2_array", usdi::AttributeType::Float2Array, v);
    }
    {
        float3 v[] = { { 1.23f, 2.34f, 3.45f } ,{ 4.56f, 5.67f, 6.78f } ,{ 7.89f, 8.90f, 9.01f} };
        AddAttribute(schema, "float3_array", usdi::AttributeType::Float3Array, v);
    }
    {
        float4 v[] = { { 1.23f, 2.34f, 3.45f, 4.56f } ,{ 5.67f, 6.78f, 7.89f, 8.90f } ,{ 9.01f, 0.12f, 1.23f, 2.34f } };
        AddAttribute(schema, "float4_array", usdi::AttributeType::Float4Array, v);
    }
    {
        quatf v[] = { { 1.23f, 2.34f, 3.45f, 4.56f } ,{ 5.67f, 6.78f, 7.89f, 8.90f } ,{ 9.01f, 0.12f, 1.23f, 2.34f } };
        AddAttribute(schema, "quaternion_array", usdi::AttributeType::QuaternionArray, v);
    }
    {
        const char *v[] = { "test_token0", "test_token1", "test_token2" };
        AddAttribute(schema, "token_array", usdi::AttributeType::TokenArray, v);
    }
    {
        const char *v[] = { "test_string0", "test_string1", "test_string2" };
        AddAttribute(schema, "string_array", usdi::AttributeType::StringArray, v);
    }

}

void TestExport(const char *filename)
{
    auto *ctx = usdiCreateContext();
    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    {
        auto *xf = usdiCreateXform(ctx, root, "Child");
        usdi::XformData data;
        usdi::Time t = 0.0;
        for (int i = 0; i < 5; ++i) {
            data.position.x += 0.2f;
            t += 1.0 / 30.0;
            usdiXformWriteSample(xf, &data, t);
        }
        TestAttributes(xf);


        {
            auto *mesh = usdiCreateMesh(ctx, xf, "TestMesh");
            float3 vertices[] = {
                { -0.5f, -0.5f, 0.0f },
                {  0.5f, -0.5f, 0.0f },
                {  0.5f,  0.5f, 0.0f },
                { -0.5f,  0.5f, 0.0f },
            };
            int counts[] = { 4 };
            int indices[] = { 0, 1, 2, 3 };

            usdi::MeshData data;
            data.points = vertices;
            data.counts = counts;
            data.indices = indices;
            data.num_points = std::extent<decltype(vertices)>::value;
            data.num_counts = std::extent<decltype(counts)>::value;
            data.num_indices = std::extent<decltype(indices)>::value;

            usdi::Time t = 0.0;
            for (int i = 0; i < 5; ++i) {
                t += 1.0 / 30.0;
                usdiMeshWriteSample(mesh, &data, t);
            }
        }
    }
    {
        auto *mesh2 = usdiCreateMesh(ctx, root, "TestRootMesh");

        int vset[] = {
            usdiPrimCreateVariantSet(mesh2, "VSet1"),
            usdiPrimCreateVariantSet(mesh2, "VSet2"),
        };
        usdiPrimCreateVariant(mesh2, vset[0], "Variant1");
        usdiPrimCreateVariant(mesh2, vset[0], "Variant2");
        usdiPrimCreateVariant(mesh2, vset[1], "Variant3");
        usdiPrimCreateVariant(mesh2, vset[1], "Variant4");
        usdiPrimCreateVariant(mesh2, vset[1], "Variant5");
        usdiPrimSetVariantSelection(mesh2, 0, 0);

        float3 vertices[] = {
            { -0.5f, -0.5f, 0.0f },
            { 0.5f, -0.5f, 0.0f },
            { 0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f },
        };
        int counts[] = { 3, 3 };
        int indices[] = { 0, 1, 2, 0, 2, 3 };

        usdi::MeshData data;
        data.points = vertices;
        data.counts = counts;
        data.indices = indices;
        data.num_points = std::extent<decltype(vertices)>::value;
        data.num_counts = std::extent<decltype(counts)>::value;
        data.num_indices = std::extent<decltype(indices)>::value;

        usdi::Time t = 0.0;
        for (int i = 0; i < 5; ++i) {
            t += 1.0 / 30.0;
            usdiMeshWriteSample(mesh2, &data, t);
        }
    }


    usdiCreateReference(ctx, "/Ref", "", "/Child");
    usdiCreateReference(ctx, "/Ref2", "", "/TestRootMesh");
    usdiFlatten(ctx);

    usdiSave(ctx);
    usdiSaveAs(ctx, "Hoge.usda");
    usdiDestroyContext(ctx);
}

void TestExportReference(const char *filename, const char *flatten)
{
    auto *ctx = usdiCreateContext();
    usdiCreateStage(ctx, filename);
    usdiCreateReference(ctx, "/Test", "TestExport.usda", "/Child");
    usdiSave(ctx);

    usdiFlatten(ctx);
    usdiSaveAs(ctx, flatten);

    usdiDestroyContext(ctx);
}
