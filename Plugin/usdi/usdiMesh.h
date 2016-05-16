#pragma once

namespace usdi {


struct MeshSample
{
    VtArray<GfVec3f> points;
    VtArray<GfVec3f> velocities;
    VtArray<GfVec3f> normals;
    VtArray<int>     counts;
    VtArray<int>     indices;
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh);
    Mesh(Context *ctx, Schema *parent, const char *name);
    ~Mesh() override;

    UsdGeomMesh&    getUSDSchema() override;

    void            getSummary(MeshSummary& dst) const;
    bool            readSample(MeshData& dst, Time t);
    bool            writeSample(const MeshData& src, Time t);

private:
    typedef MeshSample Sample;

    UsdGeomMesh         m_mesh;
    TopologyVariance    m_topology_variance = TopologyVariance::Constant;
};

} // namespace usdi
