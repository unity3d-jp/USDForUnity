#pragma once
#include "Foundation/aiMeshOps.h"

namespace usdi {

struct MeshSample
{
    VtArray<GfVec3f> points_sp;
    VtArray<GfVec3f> normals_sp;
    VtArray<GfVec4f> colors_sp;
    VtArray<GfVec2f> uv0_sp, uv1_sp;
    VtArray<GfVec3f> velocities_sp;
    VtArray<int>     counts_sp;
    VtArray<int>     indices_sp;

    MeshRefiner m_refiner;

    RawVector<float3> points;
    RawVector<float3> normals;
    RawVector<float4> tangents;
    RawVector<float2> uv0, uv1;
    RawVector<int> offsets;
    RawVector<int> indices_triangulated;
    RawVector<int> indices_flattened_triangulated;

    float3           bounds_min = {}, bounds_max = {};
    float3           center = {}, extents = {};
};


class Mesh : public Xform
{
typedef Xform super;
public:
    DefSchemaTraits(UsdGeomMesh, "Mesh");

    Mesh(Context *ctx, Schema *parent, const UsdPrim& prim);
    Mesh(Context *ctx, Schema *parent, const char *name, const char *type = _getUsdTypeName());
    ~Mesh() override;

    void                updateSample(Time t) override;

    const MeshSummary&  getSummary() const;
    bool                readSample(MeshData& dst);
    bool                writeSample(const MeshData& src, Time t);

    using SampleCallback = std::function<void(const MeshData& data, Time t)>;
    int eachSample(const SampleCallback& cb);

private:
    UsdGeomMesh         m_mesh;
    MeshSample          m_sample;
    Attribute           *m_attr_colors = nullptr;
    Attribute           *m_attr_uv0 = nullptr;

    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
    int                 m_num_indices = 0;
    int                 m_num_indices_triangulated = 0;
};

} // namespace usdi
