
#include <cstdio>
#include <vector>
#include "../usdi/usdi.h"

typedef unsigned char byte;
typedef unsigned int uint;
using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quatf;

template<class T> void P(T v);
template<> void P(byte v) { printf("0x%02X", v); }
template<> void P(int v) { printf("%d", v); }
template<> void P(uint v) { printf("%u", v); }
template<> void P(float v) { printf("%.3f", v); }
template<> void P(float2 v) { printf("(%.3f, %.3f)", v.x, v.y); }
template<> void P(float3 v) { printf("(%.3f, %.3f, %.3f)", v.x, v.y, v.z); }
template<> void P(float4 v) { printf("(%.3f, %.3f, %.3f, %.3f)", v.x, v.y, v.z, v.w); }
template<> void P(quatf v) { printf("(%.3f, %.3f, %.3f, %.3f)", v.x, v.y, v.z, v.w); }
template<> void P(const char* v) { printf("\"%s\"", v); }

template<> void P(const char** v)
{
    for (int i = 0; ; ++i) {
        if (!v[i]) { break; }
        if (i > 0) { printf(", "); }
        printf("\"%s\"", v[i]);
    }
}


template<class T>
static void P(const std::vector<T>& data)
{
    printf("[");
    auto n = data.size();
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) { printf(", "); }
        P<T>(data[i]);
    }
    printf("]");
}

static void InspectAttribute(usdi::Attribute *attr)
{

    usdi::Time t = 0.0;

#define PScalar(Enum, Type)\
    case usdi::AttributeType::Enum:\
    {\
        Type buf;\
        usdi::AttributeData data; data.data = (void*)&buf; data.num_elements = 1;\
        usdiAttrReadSample(attr, &data, t, true);\
        P(buf);\
        break;\
    }

#define PString(Enum, Type) PScalar(Enum, Type)

#define PVector(Enum, Type)\
    case usdi::AttributeType::Enum:\
    {\
        std::vector<Type> buf;\
        usdi::AttributeData data;\
        usdiAttrReadSample(attr, &data, t, true);\
        buf.resize(data.num_elements);\
        data.data = buf.data();\
        data.num_elements = buf.size();\
        usdiAttrReadSample(attr, &data, t, true); \
        P(buf);\
        break;\
    }

    usdi::AttributeSummary summary;
    usdiAttrGetSummary(attr, &summary);
    printf("    %s (%s): ", usdiAttrGetName(attr), usdiAttrGetTypeName(attr));


    switch (summary.type) {
        PScalar(Byte, byte);
        PScalar(Int, int);
        PScalar(UInt, uint);
        PScalar(Float, float);
        PScalar(Float2, float2);
        PScalar(Float3, float3);
        PScalar(Float4, float4);
        PScalar(QuatF, quatf);
        PString(Token, const char*);
        PString(String, const char*);
        PString(Asset, const char*);

        PVector(ByteArray, byte);
        PVector(IntArray, int);
        PVector(UIntArray, uint);
        PVector(FloatArray, float);
        PVector(Float2Array, float2);
        PVector(Float3Array, float3);
        PVector(Float4Array, float4);
        PVector(QuatFArray, quatf);
        PString(TokenArray, const char**);
        PString(StringArray, const char**);
        PString(AssetArray, const char**);
    }

#undef PScalar
#undef PVector

    printf("\n");
}

static void InspectRecursive(usdi::Schema *schema)
{
    if (!schema) { return; }

    printf("  %s [%p] (%s", usdiPrimGetPath(schema), schema, usdiPrimGetUsdTypeName(schema));
    if (usdiPrimIsInstanceable(schema)) {
        printf(", instanceable");
    }
    if (usdiPrimIsInstance(schema)) {
        printf(", instance of %s", usdiPrimGetPath(usdiPrimGetMaster(schema)));
    }
    if (usdiPrimIsMaster(schema)) {
        printf(", master");
    }
    if (usdiPrimHasPayload(schema)) {
        printf(", payload");
    }
    printf(")\n");

    {
        usdi::Schema *r = schema;
        if (auto master = usdiPrimGetMaster(schema)) {
            r = master;
        }
        int nattr = usdiPrimGetNumAttributes(r);
        for (int i = 0; i < nattr; ++i) {
            InspectAttribute(usdiPrimGetAttribute(r, i));
        }
    }

    int num_children = usdiPrimGetNumChildren(schema);
    for (int i = 0; i < num_children; ++i) {
        auto child = usdiPrimGetChild(schema, i);
        InspectRecursive(child);
    }
}

bool TestImport(const char *path)
{
    if (!path) { return false; }

    auto *ctx = usdiCreateContext();
    if (!usdiOpen(ctx, path)) {
        printf("failed to load %s\n", path);
    }
    else {
        for (int i = 0; i < usdiGetNumMasters(ctx); ++i) {
            InspectRecursive(usdiGetMaster(ctx, i));
        }
        InspectRecursive(usdiGetRoot(ctx));
    }

    usdiDestroyContext(ctx);

    return true;
}
