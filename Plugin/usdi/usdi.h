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


enum class TopologyVariance
{
    Constant, // both vertices and topologies are constant
    Homogenous, // vertices are not constant (= animated). topologies are constant.
    Heterogenous, // both vertices and topologies are not constant
};

struct Time
{
    double time = 0.0;
};

struct ImportConfig
{
    bool triangulate = true;
    bool swap_handedness = true;
    bool swap_faces = false;
};

struct ExportConfig
{
    bool ascii = false;
    bool swap_handedness = true;
    bool swap_faces = false;
};

struct XformData
{
    float3 position = { 0.0f, 0.0f, 0.0f};
    float4 rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // quaternion
    float3 scale = { 1.0f, 1.0f, 1.0f };
};

struct CameraData
{
    float near_clipping_plane = 0.3f;
    float far_clipping_plane = 1000.0f;
    float field_of_view = 60.0f;        // in degree. vertical one
    float aspect_ratio = 16.0f / 9.0f;

    float focus_distance = 5.0f;        // in cm
    float focal_length = 0.0f;          // in mm
    float aperture = 35.0f;             // in mm. vertical one
};


struct MeshSummary
{
    uint                peak_num_points = 0;
    uint                peak_num_counts = 0;
    uint                peak_num_indices = 0;
    uint                peak_num_indices_triangulated = 0;
    TopologyVariance    topology_variance = TopologyVariance::Constant;
    bool                has_normals = false;
    bool                has_velocities = false;
};

struct MeshData
{
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
    float3  *points = nullptr;
    float3  *velocities = nullptr;
    float3  *normals = nullptr;
    int     *counts = nullptr;
    int     *indices = nullptr;
    int     *indices_triangulated = nullptr;

    uint    num_points = 0;
    uint    num_counts = 0;
    uint    num_indices = 0;
    uint    num_indices_triangulated = 0;
};


struct PointsSummary
{
    uint                peak_num_points = 0;
    TopologyVariance    topology_variance = TopologyVariance::Constant;
    bool                has_velocities = false;
};

struct PointsData
{
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
    float3  *points = nullptr;
    float3  *velocities = nullptr;

    uint    num_points = 0;
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
usdiExport const char*      usdiGetTypeName(usdi::Schema *schema);
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

