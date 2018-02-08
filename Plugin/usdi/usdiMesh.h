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

    VtArray<float>      bone_weights;
    VtArray<int>        bone_indices;
    VtArray<GfMatrix4f> bindposes;
    VtArray<TfToken>    bones;
    VtArray<const char*> bones_;
    TfToken          root_bone;
    RawVector<Weights4> weights4;
    RawVector<Weights8> weights8;
    int              max_bone_weights = 4;

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
    bool                readSample(MeshData& dst, Time t);
    bool                writeSample(const MeshData& src, Time t);

    using SampleCallback = std::function<void(const MeshData& data, Time t)>;
    int eachSample(const SampleCallback& cb);

    // ugly workaround for C# (C# strings are need to be copied on C++ side)
    void                assignRootBone(MeshData& dst, const char *v);
    void                assignBones(MeshData& dst, const char **v, int n);

private:
    UsdGeomMesh         m_mesh;
    MeshSample          m_sample;
    Attribute           *m_attr_colors = nullptr;
    Attribute           *m_attr_uv0 = nullptr;

    // bone & weights attributes
    Attribute           *m_attr_bone_weights = nullptr;
    Attribute           *m_attr_bone_indices = nullptr;
    Attribute           *m_attr_bindposes = nullptr;
    Attribute           *m_attr_bones = nullptr;
    Attribute           *m_attr_root_bone = nullptr;
    Attribute           *m_attr_max_bone_weights = nullptr;

    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
    int                 m_num_indices = 0;
    int                 m_num_indices_triangulated = 0;
};

} // namespace usdi
