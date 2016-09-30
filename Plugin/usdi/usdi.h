#pragma once

#define usdiCLinkage extern "C"
#ifdef _WIN32
    #ifndef usdiStaticLink
        #ifdef usdiImpl
            #define usdiAPI __declspec(dllexport)
        #else
            #define usdiAPI __declspec(dllimport)
        #endif
    #else
        #define usdiAPI
    #endif
#else
    #define usdiAPI
#endif

namespace usdi {

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned int handle_t; // async handle
#ifndef usdiImpl
    // force make convertible
    class Context {};
    class Attribute {};
    class Schema {};
    class Xform : public Schema  {};
    class Camera : public Xform {};
    class Mesh : public Xform {};
    class Points : public Xform {};

    struct float2 { float x, y; };
    struct float3 { float x, y, z; };
    struct float4 { float x, y, z, w; };
    struct quatf { float x, y, z, w; };
    struct float3x3 { float3 v[3]; };
    struct float4x4 { float4 v[4]; };
#endif


enum class InterpolationType
{
    None,
    Linear,
};

enum class NormalCalculationType
{
    Never,
    WhenMissing,
    Always,
};

enum class AttributeType
{
    Unknown,
    Bool,
    Byte,
    Int,
    UInt,
    Float,
    Float2,
    Float3,
    Float4,
    Quaternion,
    Token,
    String,
    Asset,
    UnknownArray = 0x100,
    BoolArray,
    ByteArray,
    IntArray,
    UIntArray,
    FloatArray,
    Float2Array,
    Float3Array,
    Float4Array,
    QuaternionArray,
    TokenArray,
    StringArray,
    AssetArray,
};

enum class TopologyVariance
{
    Constant, // both vertices and topologies are constant
    Homogenous, // vertices are not constant (= animated). topologies are constant.
    Heterogenous, // both vertices and topologies are not constant
};

typedef double Time;

struct ImportConfig
{
    InterpolationType interpolation = InterpolationType::Linear;
    NormalCalculationType normal_calculation = NormalCalculationType::WhenMissing;
    float scale = 1.0f;
    bool triangulate = true;
    bool swap_handedness = true;
    bool swap_faces = true;
    bool split_mesh = false;
    bool double_buffering = false;
};

struct ExportConfig
{
    float scale = 1.0f;
    bool swap_handedness = true;
    bool swap_faces = true;
};


struct XformSummary
{
    enum class Type {
        Unknown,
        TRS,
        Matrix,
    };

    Time start = 0.0, end = 0.0;
    Type type = Type::Unknown;
};

struct XformData
{
    enum class Flags {
        UpdatedMask     = 0xf,
        UpdatedPosition = 0x1,
        UpdatedRotation = 0x2,
        UpdatedScale    = 0x4,
    };

    int flags = 0;
    float3 position = { 0.0f, 0.0f, 0.0f};
    quatf rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    float3 scale = { 1.0f, 1.0f, 1.0f };
    float4x4 transform = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
};


struct CameraSummary
{
    Time start = 0.0, end = 0.0;
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
    Time                start = 0.0, end = 0.0;
    TopologyVariance    topology_variance = TopologyVariance::Constant;
    bool                has_normals = false;
    bool                has_uvs = false;
    bool                has_velocities = false;
};

struct SplitedMeshData
{
    float3  *points = nullptr;
    float3  *normals = nullptr;
    float2  *uvs = nullptr;
    int     *indices = nullptr;
    uint    num_points = 0;

    float3  center = { 0.0f, 0.0f, 0.0f };
    float3  extents = { 0.0f, 0.0f, 0.0f };
};

struct MeshData
{
    // these pointers can be null (in this case, just be ignored).
    // otherwise, if you pass to usdiMeshSampleReadData(), pointers must point valid memory block to store data.
    float3  *points = nullptr;
    float3  *velocities = nullptr;
    float3  *normals = nullptr;
    float2  *uvs = nullptr;
    int     *counts = nullptr;
    int     *indices = nullptr;
    int     *indices_triangulated = nullptr;

    SplitedMeshData *splits = nullptr;

    uint    num_points = 0;
    uint    num_counts = 0;
    uint    num_indices = 0;
    uint    num_indices_triangulated = 0;
    uint    num_splits = 0;

    float3  center = { 0.0f, 0.0f, 0.0f };
    float3  extents = { 0.0f, 0.0f, 0.0f };
};


struct PointsSummary
{
    Time    start = 0.0, end = 0.0;
    uint    peak_num_points = 0;
    bool    has_velocities = false;
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

usdiAPI void             usdiSetDebugLevel(int l);
usdiAPI usdi::Time       usdiDefaultTime();

usdiAPI void             usdiSetPluginPath(const char *path);

// Context interface
usdiAPI usdi::Context*   usdiCreateContext();
usdiAPI void             usdiDestroyContext(usdi::Context *ctx);
usdiAPI bool             usdiOpen(usdi::Context *ctx, const char *path);
usdiAPI bool             usdiCreateStage(usdi::Context *ctx, const char *path);
usdiAPI bool             usdiSave(usdi::Context *ctx);
usdiAPI bool             usdiWrite(usdi::Context *ctx, const char *path);
usdiAPI void             usdiSetImportConfig(usdi::Context *ctx, const usdi::ImportConfig *conf);
usdiAPI void             usdiGetImportConfig(usdi::Context *ctx, usdi::ImportConfig *conf);
usdiAPI void             usdiSetExportConfig(usdi::Context *ctx, const usdi::ExportConfig *conf);
usdiAPI void             usdiGetExportConfig(usdi::Context *ctx, usdi::ExportConfig *conf);
usdiAPI usdi::Schema*    usdiGetRoot(usdi::Context *ctx);
usdiAPI void             usdiUpdateAllSamples(usdi::Context *ctx, usdi::Time t);

// Schema interface
usdiAPI int              usdiGetID(usdi::Schema *schema);
usdiAPI const char*      usdiGetPath(usdi::Schema *schema);
usdiAPI const char*      usdiGetName(usdi::Schema *schema);
usdiAPI const char*      usdiGetTypeName(usdi::Schema *schema);
usdiAPI usdi::Schema*    usdiGetParent(usdi::Schema *schema);
usdiAPI int              usdiGetNumChildren(usdi::Schema *schema);
usdiAPI usdi::Schema*    usdiGetChild(usdi::Schema *schema, int i);
usdiAPI int              usdiGetNumAttributes(usdi::Schema *schema);
usdiAPI usdi::Attribute* usdiGetAttribute(usdi::Schema *schema, int i);
usdiAPI usdi::Attribute* usdiFindAttribute(usdi::Schema *schema, const char *name);
usdiAPI usdi::Attribute* usdiCreateAttribute(usdi::Schema *schema, const char *name, usdi::AttributeType type);
usdiAPI bool             usdiNeedsUpdate(usdi::Schema *schema);

// Xform interface
usdiAPI usdi::Xform*     usdiAsXform(usdi::Schema *schema); // dynamic cast to Xform
usdiAPI usdi::Xform*     usdiCreateXform(usdi::Context *ctx, usdi::Schema *parent, const char *name);
usdiAPI void             usdiXformGetSummary(usdi::Xform *xf, usdi::XformSummary *dst);
usdiAPI bool             usdiXformReadSample(usdi::Xform *xf, usdi::XformData *dst, usdi::Time t);
usdiAPI bool             usdiXformWriteSample(usdi::Xform *xf, const usdi::XformData *src, usdi::Time t);

// Camera interface
usdiAPI usdi::Camera*    usdiAsCamera(usdi::Schema *schema); // dynamic cast to Camera
usdiAPI usdi::Camera*    usdiCreateCamera(usdi::Context *ctx, usdi::Schema *parent, const char *name);
usdiAPI void             usdiCameraGetSummary(usdi::Camera *cam, usdi::CameraSummary *dst);
usdiAPI bool             usdiCameraReadSample(usdi::Camera *cam, usdi::CameraData *dst, usdi::Time t);
usdiAPI bool             usdiCameraWriteSample(usdi::Camera *cam, const usdi::CameraData *src, usdi::Time t);

// Mesh interface
usdiAPI usdi::Mesh*      usdiAsMesh(usdi::Schema *schema); // dynamic cast to Mesh
usdiAPI usdi::Mesh*      usdiCreateMesh(usdi::Context *ctx, usdi::Schema *parent, const char *name);
usdiAPI void             usdiMeshGetSummary(usdi::Mesh *mesh, usdi::MeshSummary *dst);
usdiAPI bool             usdiMeshReadSample(usdi::Mesh *mesh, usdi::MeshData *dst, usdi::Time t, bool copy);
usdiAPI bool             usdiMeshWriteSample(usdi::Mesh *mesh, const usdi::MeshData *src, usdi::Time t);

// Points interface
usdiAPI usdi::Points*    usdiAsPoints(usdi::Schema *schema); // dynamic cast to Points
usdiAPI usdi::Points*    usdiCreatePoints(usdi::Context *ctx, usdi::Schema *parent, const char *name);
usdiAPI void             usdiPointsGetSummary(usdi::Points *points, usdi::PointsSummary *dst);
usdiAPI bool             usdiPointsReadSample(usdi::Points *points, usdi::PointsData *dst, usdi::Time t, bool copy);
usdiAPI bool             usdiPointsWriteSample(usdi::Points *points, const usdi::PointsData *src, usdi::Time t);

// Attribute interface
usdiAPI usdi::Schema*        usdiAttrGetParent(usdi::Attribute *attr);
usdiAPI const char*          usdiAttrGetName(usdi::Attribute *attr);
usdiAPI const char*          usdiAttrGetTypeName(usdi::Attribute *attr);
usdiAPI usdi::AttributeType  usdiAttrGetType(usdi::Attribute *attr);
usdiAPI int                  usdiAttrGetNumSamples(usdi::Attribute *attr);
usdiAPI int                  usdiAttrGetArraySize(usdi::Attribute *attr, usdi::Time t); // always 1 if attr is scalar
usdiAPI bool                 usdiAttrReadSample(usdi::Attribute *attr, void *dst, usdi::Time t);
usdiAPI bool                 usdiAttrReadArraySample(usdi::Attribute *attr, void *dst, int size, usdi::Time t);
usdiAPI bool                 usdiAttrWriteSample(usdi::Attribute *attr, const void *src, usdi::Time t);
usdiAPI bool                 usdiAttrWriteArraySample(usdi::Attribute *attr, const void *src, int size, usdi::Time t);

} // extern "C"




// ext

namespace usdi {
    struct MapContext;
    typedef void(*TaskFunc)(void*);
} // namespace usdi

extern "C" {

usdiAPI void            usdiExtVtxTaskQueue(const usdi::MeshData *src, usdi::MapContext *ctxVB, usdi::MapContext *ctxIB);
usdiAPI void            usdiExtVtxTaskEndQueing();
usdiAPI void            usdiExtVtxTaskFlush();
usdiAPI void            usdiExtVtxTaskClear();

usdiAPI usdi::handle_t  usdiExtTaskCreate(usdi::TaskFunc func, void *arg, const char *dbg_name);
usdiAPI void            usdiExtTaskDestroy(usdi::handle_t h);
usdiAPI void            usdiExtTaskRun(usdi::handle_t h);
usdiAPI bool            usdiExtTaskIsRunning(usdi::handle_t h);
usdiAPI void            usdiExtTaskWait(usdi::handle_t h);

} // extern "C"

