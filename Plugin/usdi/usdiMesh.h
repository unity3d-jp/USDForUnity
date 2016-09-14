#pragma once

namespace usdi {


struct MeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
    VtArray<GfVec3f> normals;
    VtArray<GfVec2f> uvs;
    VtArray<int>     counts;
    VtArray<int>     indices;

    void clear();
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh);
    Mesh(Context *ctx, Schema *parent, const char *name);
    ~Mesh() override;

    UsdGeomMesh&        getUSDSchema() override;

    const MeshSummary&  getSummary() const;
    bool                readSample(MeshData& dst, Time t);
    bool                writeSample(const MeshData& src, Time t);

private:
    void updateSummary() const;

private:
    UsdGeomMesh         m_mesh;
    MeshSample          m_sample;
    Attribute           *m_attr_uv = nullptr;

    mutable Time        m_prev_time = DBL_MIN;
    mutable bool        m_summary_needs_update = true;
    mutable MeshSummary m_summary;
};

} // namespace usdi
