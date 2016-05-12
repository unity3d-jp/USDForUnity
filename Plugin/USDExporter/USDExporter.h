#pragma once

#define usdeCLinkage extern "C"
#ifdef _WIN32
    #ifndef usdeStaticLink
        #ifdef usdeImpl
            #define usdeExport __declspec(dllexport)
        #else
            #define usdeExport __declspec(dllimport)
        #endif
    #else
        #define usdeExport
    #endif
#else
    #define usdeExport
#endif

namespace usde {

class ExportContext;

class Schema;
class Xform;
class Camera;
class PolyMesh;

class Sample;
class XformSample;
class CameraSample;
class PolyMeshSample;


struct float2
{
    float x, y;

    float2() {}
    float2(float _x, float _y) : x(_x), y(_y) {}
};

struct float3
{
    float x, y, z;

    float3() {}
    float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct float4
{
    float x, y, z, w;

    float4() {}
    float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), w(_w) {}
};

} // namespace usde
