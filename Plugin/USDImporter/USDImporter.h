#pragma once

#define usdiCLinkage extern "C"
#ifdef _WIN32
    #ifndef usdiStaticLink
        #ifdef usdiImpl
            #define usdiExport __declspec(dllexport)
        #else
            #define usdiExport __declspec(dllimport)
        #endif
    #else
        #define usdiExport
    #endif
#else
    #define usdiExport
#endif

namespace usdi {

class ImportContext;

class Schema;
class Xform;
class Camera;
class Mesh;

class Sample;
class XformSample;
class CameraSample;
class MeshSample;


typedef unsigned int uint32;

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


enum class TopologyVariance
{
    Constant, // both vertices and topologies are constant
    Homogenous, // vertices are not constant (= animated). topologies are constant.
    Heterogenous, // both vertices and topologies are not constant
};

struct Time
{
    double time;
    uint32 index;
};

struct XformData
{
    float3 position;
    float4 rotation;
    float3 scale;
};

struct CameraData
{
};

struct MeshData
{
    // these pointers can be null (just be ignored). otherwise:
    // if you pass to usdiPolyMeshSampleCopyData(), pointers must point valid memory block to store data.
    float3  *points;
    float3  *normals;
    int     *face_vertex_counts;
    int     *face_vertex_indices;

    uint32  num_points;
    uint32  num_face_vertex_counts;
    uint32  num_face_vertex_indices;
};

} // namespace usdi

extern "C" {

usdiExport usdi::ImportContext*     usdiOpen(const char *path);

usdiExport usdi::Schema*            usdiGetRoot(usdi::ImportContext *ctx);
usdiExport int                      usdiGetNumChildren(usdi::Schema *schema);
usdiExport usdi::Schema*            usdiGetChild(usdi::Schema *schema, int i);

usdiExport usdi::Xform*             usdiAsXform(usdi::Schema *schema);
usdiExport usdi::XformSample*       usdiXformGetSample(usdi::Xform *xf, usdi::Time t);
usdiExport void                     usdiXformGetData(usdi::XformSample *sample, usdi::XformData *data);

usdiExport usdi::Camera*            usdiAsCamera(usdi::Schema *schema);
usdiExport usdi::CameraSample*      usdiCameraGetSample(usdi::Camera *cam, usdi::Time t);
usdiExport void                     usdiCameraGetData(usdi::CameraSample *sample, usdi::CameraData *data);

usdiExport usdi::Mesh*              usdiAsMesh(usdi::Schema *schema);
usdiExport usdi::MeshSample*        usdiMeshGetSample(usdi::Mesh *mesh, usdi::Time t);
usdiExport void                     usdiMeshSampleCopyData(usdi::MeshSample *sample, usdi::MeshData *data);

} // extern "C"

