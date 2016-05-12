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
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
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
usdiExport void                     usdiXformReadData(usdi::Xform *xf, usdi::Time t, usdi::XformData *dst);
usdiExport void                     usdiXformWriteData(usdi::Xform *xf, usdi::Time t, const usdi::XformData *src);

usdiExport usdi::Camera*            usdiAsCamera(usdi::Schema *schema);
usdiExport void                     usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst);
usdiExport void                     usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src);

usdiExport usdi::Mesh*              usdiAsMesh(usdi::Schema *schema);
usdiExport void                     usdiMeshReadSample(usdi::Mesh *mesh, usdi::Time t, usdi::MeshData *dst);
usdiExport void                     usdiMeshWriteSample(usdi::Mesh *mesh, usdi::Time t, const usdi::MeshData *src);

} // extern "C"

