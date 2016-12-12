#pragma once

namespace usdi {


struct SubmeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> normals;
    VtArray<GfVec4f> tangents;
    VtArray<GfVec2f> uvs;
    VtArray<int>     indices;
    float3           bounds_min = {}, bounds_max = {};
    float3           center = {}, extents = {};

    void clear();
};

struct MeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
    VtArray<GfVec3f> normals;
    VtArray<GfVec4f> tangents;
    VtArray<GfVec2f> uvs;
    VtArray<int>     counts;
    VtArray<int>     offsets;
    VtArray<int>     indices;
    VtArray<int>     indices_triangulated;

    VtArray<float>   bone_weights;
    VtArray<int>     bone_indices;
    VtArray<TfToken> bones;
    VtArray<const char*> bones_;
    TfToken          root_bone;
    VtArray<Weights4> weights4;
    VtArray<Weights8> weights8;
    int              max_bone_weight = 4;

    float3           bounds_min = {}, bounds_max = {};
    float3           center = {}, extents = {};

    void clear();
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
    bool                readSample(MeshData& dst, Time t, bool copy);
    bool                writeSample(const MeshData& src, Time t);

    // ugly workaround for C#
    void                assignRootBone(MeshData& dst, const char *v);
    void                assignBones(MeshData& dst, const char **v, int n);

private:
    typedef std::vector<SubmeshSample> SubmeshSamples;

    UsdGeomMesh         m_mesh;
    MeshSample          m_sample[2], *m_front_sample = nullptr;
    SubmeshSamples      m_submeshes[2], *m_front_submesh = nullptr;
    Attribute           *m_attr_uv = nullptr;
    Attribute           *m_attr_tangents = nullptr;

    // bone & weights attributes
    Attribute           *m_attr_bone_weights = nullptr;
    Attribute           *m_attr_bone_indices = nullptr;
    Attribute           *m_attr_bones = nullptr;
    Attribute           *m_attr_root_bone = nullptr;
    Attribute           *m_attr_max_bone_weight = nullptr;

    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
    mutable int         m_num_indices = 0;
    mutable int         m_num_indices_triangulated = 0;
};

} // namespace usdi
