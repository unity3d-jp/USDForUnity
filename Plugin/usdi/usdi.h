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

class Context;
class Schema;
class Xform;
class Camera;
class Mesh;
class Points;


typedef unsigned int uint;
struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };


enum class SchemaType
{
    Unknown,
    Xform,
    Camera,
    Mesh,
    Points,
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

struct ImportConfig
{
    bool triangulate;
    bool swap_handedness;
    bool swap_faces;

    ImportConfig()
        : triangulate(true)
        , swap_handedness(true)
        , swap_faces(false)
    {}
};

struct ExportConfig
{
    bool ascii;
    bool swap_handedness;
    bool swap_faces;

    ExportConfig()
        : ascii(false)
        , swap_handedness(true)
        , swap_faces(false)
    {}
};

struct XformData
{
    float3 position;
    float4 rotation; // quaternion
    float3 scale;
};

struct CameraData
{
};


struct MeshSummary
{
    uint                peak_num_points;
    uint                peak_num_counts;
    uint                peak_num_indices;
    uint                peak_num_indices_triangulated;
    TopologyVariance    topology_variance;
    bool                has_normals;
    bool                has_velocities;
};

struct MeshData
{
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
    float3  *points;
    float3  *velocities;
    float3  *normals;
    int     *counts;
    int     *indices;
    int     *indices_triangulated;

    uint    num_points;
    uint    num_counts;
    uint    num_indices;
    uint    num_indices_triangulated;
};


struct PointsSummary
{
    uint                peak_num_points;
    TopologyVariance    topology_variance;
    bool                has_velocities;
};

struct PointsData
{
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
    float3  *points;
    float3  *velocities;

    uint    num_points;
};

} // namespace usdi

extern "C" {

usdiExport usdi::Context*   usdiOpen(const char *path);
usdiExport usdi::Context*   usdiCreateContext(const char *identifier);
usdiExport void             usdiDestroyContext(usdi::Context *ctx);
usdiExport bool             usdiWrite(usdi::Context *ctx, const char *path);
usdiExport void             usdiSetImportConfig(usdi::Context *ctx, const usdi::ImportConfig *conf);
usdiExport void             usdiGetImportConfig(usdi::Context *ctx, usdi::ImportConfig *conf);
usdiExport void             usdiSetExportConfig(usdi::Context *ctx, const usdi::ExportConfig *conf);
usdiExport void             usdiGetExportConfig(usdi::Context *ctx, usdi::ExportConfig *conf);
usdiExport usdi::Schema*    usdiGetRoot(usdi::Context *ctx);

usdiExport int              usdiGetID(usdi::Schema *schema);
usdiExport const char*      usdiGetPath(usdi::Schema *schema);
usdiExport const char*      usdiGetName(usdi::Schema *schema);
usdiExport usdi::SchemaType usdiGetType(usdi::Schema *schema);
usdiExport usdi::Schema*    usdiGetParent(usdi::Schema *schema);
usdiExport int              usdiGetNumChildren(usdi::Schema *schema);
usdiExport usdi::Schema*    usdiGetChild(usdi::Schema *schema, int i);

usdiExport usdi::Xform*     usdiAsXform(usdi::Schema *schema);
usdiExport usdi::Xform*     usdiCreateXform(usdi::Schema *parent, const char *name);
usdiExport bool             usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t);
usdiExport bool             usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t);

usdiExport usdi::Camera*    usdiAsCamera(usdi::Schema *schema);
usdiExport usdi::Camera*    usdiCreateCamera(usdi::Schema *parent, const char *name);
usdiExport bool             usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst, usdi::Time t);
usdiExport bool             usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src, usdi::Time t);

usdiExport usdi::Mesh*      usdiAsMesh(usdi::Schema *schema);
usdiExport usdi::Mesh*      usdiCreateMesh(usdi::Schema *parent, const char *name);
usdiExport void             usdiMeshGetSummary(usdi::Mesh *mesh, usdi::MeshSummary *dst);
usdiExport bool             usdiMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t);
usdiExport bool             usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t);

usdiExport usdi::Points*    usdiAsPoints(usdi::Schema *schema);
usdiExport usdi::Points*    usdiCreatePoints(usdi::Schema *parent, const char *name);
usdiExport void             usdiPointsGetSummary(usdi::Points *points, usdi::PointsSummary *dst);
usdiExport bool             usdiPointsReadSample(usdi::Points *points, usdi::PointsData *dst, usdi::Time t);
usdiExport bool             usdiPointsWriteSample(usdi::Points *points, const usdi::PointsData *src, usdi::Time t);

} // extern "C"

