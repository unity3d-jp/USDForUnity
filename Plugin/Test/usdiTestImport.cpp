#include <cstdio>
#include <vector>
#include "../usdi/usdi.h"

typedef unsigned char byte;
typedef unsigned int uint;
using usdi::float2;
using usdi::float3;
using usdi::float4;
using usdi::quaternion;

template<class T> void P(T v);
template<> void P<byte>(byte v) { printf("0x%02X", v); }
template<> void P<int>(int v) { printf("%d", v); }
template<> void P<uint>(uint v) { printf("%u", v); }
template<> void P<float>(float v) { printf("%.3f", v); }
template<> void P<float2>(float2 v) { printf("(%.3f, %.3f)", v.x, v.y); }
template<> void P<float3>(float3 v) { printf("(%.3f, %.3f, %.3f)", v.x, v.y, v.z); }
template<> void P<float4>(float4 v) { printf("(%.3f, %.3f, %.3f, %.3f)", v.x, v.y, v.z, v.w); }
template<> void P<quaternion>(quaternion v) { printf("(%.3f, %.3f, %.3f, %.3f)", v.x, v.y, v.z, v.w); }
template<> void P<const char*>(const char* v) { printf("\"%s\"", v); }


template<class T>
void P(const std::vector<T>& data)
{
    printf("[");
    auto n = data.size();
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) { printf(", "); }
        P<T>(data[i]);
    }
    printf("]");
}

void InspectAttribute(usdi::Attribute *attr)
{
    printf("    %s (%s): ", usdiAttrGetName(attr), usdiAttrGetTypeName(attr));

    usdi::Time t = 0.0;

    byte buf_byte;
    int buf_int;
    uint buf_uint;
    float buf_float;
    float2 buf_float2;
    float3 buf_float3;
    float4 buf_float4;
    quaternion buf_quat;
    const char *buf_str;

    std::vector<byte> buf_byte_array;
    std::vector<int> buf_int_array;
    std::vector<uint> buf_uint_array;
    std::vector<float> buf_float_array;
    std::vector<float2> buf_float2_array;
    std::vector<float3> buf_float3_array;
    std::vector<float4> buf_float4_array;
    std::vector<quaternion> buf_quat_array;
    std::vector<const char*> buf_str_array;

    int nsize = usdiAttrGetArraySize(attr, t);

    switch (usdiAttrGetType(attr)) {
    case usdi::AttributeType::Byte:
        usdiAttrReadSample(attr, &buf_byte, t);
        P(buf_byte);
        break;
    case usdi::AttributeType::Int:
        usdiAttrReadSample(attr, &buf_int, t);
        P(buf_int);
        break;
    case usdi::AttributeType::UInt:
        usdiAttrReadSample(attr, &buf_uint, t);
        P(buf_uint);
        break;
    case usdi::AttributeType::Float:
        usdiAttrReadSample(attr, &buf_float, t);
        P(buf_float);
        break;
    case usdi::AttributeType::Float2:
        usdiAttrReadSample(attr, &buf_float2, t);
        P(buf_float2);
        break;
    case usdi::AttributeType::Float3:
        usdiAttrReadSample(attr, &buf_float3, t);
        P(buf_float3);
        break;
    case usdi::AttributeType::Float4:
        usdiAttrReadSample(attr, &buf_float4, t);
        P(buf_float4);
        break;
    case usdi::AttributeType::Quaternion:
        usdiAttrReadSample(attr, &buf_quat, t);
        P(buf_quat);
        break;
    case usdi::AttributeType::Token:
        usdiAttrReadSample(attr, &buf_str, t);
        P(buf_str);
        break;
    case usdi::AttributeType::String:
        usdiAttrReadSample(attr, &buf_str, t);
        P(buf_str);
        break;

    case usdi::AttributeType::ByteArray:
        buf_byte_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_byte_array[0], nsize, t);
        P(buf_byte_array);
        break;
    case usdi::AttributeType::IntArray:
        buf_int_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_int_array[0], nsize, t);
        P(buf_int_array);
        break;
    case usdi::AttributeType::UIntArray:
        buf_uint_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_uint_array[0], nsize, t);
        P(buf_uint_array);
        break;
    case usdi::AttributeType::FloatArray:
        buf_float_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_float_array[0], nsize, t);
        P(buf_float_array);
        break;
    case usdi::AttributeType::Float2Array:
        buf_float2_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_float2_array[0], nsize, t);
        P(buf_float2_array);
        break;
    case usdi::AttributeType::Float3Array:
        buf_float3_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_float3_array[0], nsize, t);
        P(buf_float3_array);
        break;
    case usdi::AttributeType::Float4Array:
        buf_float4_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_float4_array[0], nsize, t);
        P(buf_float4_array);
        break;
    case usdi::AttributeType::QuaternionArray:
        buf_quat_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_quat_array[0], nsize, t);
        P(buf_quat_array);
        break;
    case usdi::AttributeType::TokenArray:
        buf_str_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_str_array[0], nsize, t);
        P(buf_str_array);
        break;
    case usdi::AttributeType::StringArray:
        buf_str_array.resize(nsize);
        usdiAttrReadArraySample(attr, &buf_str_array[0], nsize, t);
        P(buf_str_array);
        break;
    }
    printf("\n");
}

void InspectRecursive(usdi::Schema *schema)
{
    if (!schema) { return; }

    printf("  %s (%s)\n", usdiGetPath(schema), usdiGetTypeName(schema));
    {
        int nattr = usdiGetNumAttributes(schema);
        for (int i = 0; i < nattr; ++i) {
            InspectAttribute(usdiGetAttribute(schema, i));
        }
    }

    int num_children = usdiGetNumChildren(schema);
    for (int i = 0; i < num_children; ++i) {
        auto child = usdiGetChild(schema, i);
        InspectRecursive(child);
    }
}

bool TestImport(const char *path)
{
    auto *ctx = usdiCreateContext();
    if (!usdiOpen(ctx, path)) {
        printf("failed to load %s\n", path);
    }
    else {
        InspectRecursive(usdiGetRoot(ctx));
    }
    usdiDestroyContext(ctx);

    return true;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("first argument must be path to usd file\n");
        return 0;
    }
    TestImport(argv[1]);
}
