#pragma once

namespace usdi {


struct SubmeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> normals;
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
    VtArray<GfVec2f> uvs;
    VtArray<int>     counts;
    VtArray<int>     indices;
    VtArray<int>     indices_triangulated;
    float3           bounds_min = {}, bounds_max = {};
    float3           center = {}, extents = {};

    void clear();
};


class Mesh : public Xform
{
typedef Xform super;
friend class Context;
protected:
    Mesh(Context *ctx, Schema *parent, const UsdPrim& prim);
    Mesh(Context *ctx, Schema *parent, const char *name, const char *type = UsdTypeName);
    ~Mesh() override;

public:
    using UsdType = UsdGeomMesh;
    static const char *UsdTypeName;

    void                updateSample(Time t) override;
    void                invalidateSample() override;

    const MeshSummary&  getSummary() const;
    bool                readSample(MeshData& dst, Time t, bool copy);
    bool                writeSample(const MeshData& src, Time t);

private:
    typedef std::vector<SubmeshSample> SubmeshSamples;

    UsdGeomMesh         m_mesh;
    MeshSample          m_sample[2], *m_front_sample = nullptr;
    SubmeshSamples      m_submeshes[2], *m_front_submesh = nullptr;
    Attribute           *m_attr_uv = nullptr;

    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
    mutable int         m_num_indices = 0;
    mutable int         m_num_indices_triangulated = 0;
};

} // namespace usdi
