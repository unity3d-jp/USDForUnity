#include <type_traits>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include "../usdi/usdi.h"

using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;

template<class T> void* ToPtr(const T& v) { return (void*)&v; }
template<class T> void* ToPtr(const T* v) { return (void*)v; }


static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const char* v)
{
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = (void*)v;
    data.num_elements = 1;
    usdiAttrWriteSample(attr, &data, usdiDefaultTime());
}

template<class T>
static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const T& v)
{
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = ToPtr(v);
    data.num_elements = 1;
    usdiAttrWriteSample(attr, &data, usdiDefaultTime());
}
template<class T, size_t N>
static void AddAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type, const T (&v)[N])
{
    auto *attr = usdiPrimCreateAttribute(schema, name, type);

    usdi::AttributeData data;
    data.data = (void*)v;
    data.num_elements = N;
    for (int i = 0; i < N; ++i) {
        usdi::Time t = 1.0 / 30.0 * i;
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
        AddAttribute(schema, "quatf_scalar", usdi::AttributeType::QuatF, v);
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
        AddAttribute(schema, "quatf_array", usdi::AttributeType::QuatFArray, v);
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

    usdi::ExportSettings settings;
    settings.instanceable_by_default = true;
    usdiSetExportSettings(ctx, &settings);

    usdiCreateStage(ctx, filename);
    auto *root = usdiGetRoot(ctx);

    {
        auto *xf = usdiCreateXform(ctx, root, "Child");
        for (int i = 0; i < 5; ++i) {
            usdi::Time t = (1.0 / 30.0) * i;
            usdi::XformData data;
            data.position.x = 0.2f * i;
            usdiXformWriteSample(xf, &data, t);
        }
        TestAttributes(xf);


        {
            auto *mesh = usdiCreateMesh(ctx, xf, "TestMesh");
            float3 points[] = {
                { -0.5f, -0.5f, 0.0f },
                {  0.5f, -0.5f, 0.0f },
                {  0.5f,  0.5f, 0.0f },
                { -0.5f,  0.5f, 0.0f },
            };
            float3 normals[] = {
                { 0.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f },
            };
            float4 tangents[] = {
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
            };
            float2 uvs[] = {
                { 0.0f, 0.0f },
                { 1.0f, 0.0f },
                { 1.0f, 1.0f },
                { 0.0f, 1.0f },
            };
            int counts[] = { 4 };
            int indices[] = { 0, 1, 2, 3 };

            usdi::MeshData data;
            data.points = points;
            data.normals = normals;
            data.tangents = tangents;
            data.uvs = uvs;
            data.indices = indices;
            data.num_points = std::extent<decltype(points)>::value;
            data.num_counts = std::extent<decltype(counts)>::value;
            data.num_indices = std::extent<decltype(indices)>::value;

            for (int i = 0; i < 5; ++i) {
                usdi::Time t = (1.0 / 30.0) * i;
                usdiMeshWriteSample(mesh, &data, t);
            }
        }
    }
    {
        auto *mesh2 = usdiCreateMesh(ctx, root, "TestRootMesh");
        float3 vertices[] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
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

        for (int i = 0; i < 5; ++i) {
            usdi::Time t = (1.0 / 30.0) * i;
            usdiMeshWriteSample(mesh2, &data, t);
        }

        AddAttribute(mesh2, "TestImageAsset", usdi::AttributeType::Asset, "USDAssets/test.exr");
        AddAttribute(mesh2, "TestFBXAsset", usdi::AttributeType::Asset, "USDAssets/test.fbx");
        AddAttribute(mesh2, "TestMDLAsset", usdi::AttributeType::Asset, "USDAssets/test.mdl");
    }
    {
        auto CreateTestXformTree = [](usdi::Context *ctx, usdi::Schema *parent, std::vector<std::string> names)
        {
            for (auto& name : names) {
                parent = usdiCreateXform(ctx, parent, name.c_str());
            }
        };

        auto *xf = usdiCreateXform(ctx, root, "TestVariants");

        int vset[] = {
            usdiPrimCreateVariantSet(xf, "VariantSet0"),
            usdiPrimCreateVariantSet(xf, "VariantSet1"),
        };
        usdiPrimCreateVariant(xf, vset[0], "Variant0-0");
        usdiPrimCreateVariant(xf, vset[0], "Variant0-1");
        usdiPrimCreateVariant(xf, vset[1], "Variant1-0");
        usdiPrimCreateVariant(xf, vset[1], "Variant1-1");
        usdiPrimCreateVariant(xf, vset[1], "Variant1-2");

        usdiPrimBeginEditVariant(xf, 0, 0);
        xf = usdiAsXform(usdiFindSchema(ctx, "/TestVariants"));
        for (int i = 0; i < 5; ++i) {
            usdi::Time t = (1.0 / 30.0) * i;
            usdi::XformData data;
            data.position.x = 0.2f * i;
            usdiXformWriteSample(xf, &data, t);
        }
        CreateTestXformTree(ctx, xf, { "Variant0-0", "Hoge" });
        usdiPrimEndEditVariant(xf);

        usdiPrimBeginEditVariant(xf, 1, 1);
        xf = usdiAsXform(usdiFindSchema(ctx, "/TestVariants"));
        for (int i = 0; i < 5; ++i) {
            usdi::Time t = (1.0 / 30.0) * i;
            usdi::XformData data;
            data.position.y = 0.4f * i;
            usdiXformWriteSample(xf, &data, t);
        }
        CreateTestXformTree(ctx, xf, { "Variant1-1", "Hage", "Hige" });
        usdiPrimEndEditVariant(xf);
    }

    // create internal reference
    auto *ref = usdiCreateOverride(ctx, "/Ref");
    usdiPrimAddReference(ref, nullptr, "/Child");

    // create payload
    auto *pl = usdiCreateOverride(ctx, "/TestPayload");
    usdiPrimSetPayload(pl, "HelloWorld.usda", "/hello");


    usdiSave(ctx);

    usdiFlatten(ctx);
    usdiSaveAs(ctx, "Hoge.usda");

    usdiDestroyContext(ctx);
}

void TestExportReference(const char *filename, const char *flatten)
{
    auto *ctx = usdiCreateContext();
    usdiCreateStage(ctx, filename);

    // create external reference
    auto *ref = usdiCreateOverride(ctx, "/Test");
    usdiPrimAddReference(ref, "TestExport.usda", "/Child");

    usdiSave(ctx);

    usdiFlatten(ctx);
    usdiSaveAs(ctx, flatten);

    usdiDestroyContext(ctx);
}
