#pragma once

namespace usdi {


struct MeshSample
{
public:
    MeshSample();
    bool read(UsdGeomMesh& mesh, Time t);
    bool write(UsdGeomMesh& mesh, Time t);

    VtArray<GfVec3f> points;
    VtArray<GfVec3f> normals;
    VtArray<int>     counts;
    VtArray<int>     indices;
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh);
    ~Mesh() override;

    UsdGeomMesh&    getUSDType() override;
    SchemaType      getType() const override;

    void            getSummary(MeshSummary& dst) const;
    bool            readSample(MeshData& dst, Time t);
    bool            writeSample(const MeshData& src, Time t);

private:
    typedef MeshSample Sample;
    typedef std::unique_ptr<Sample> SamplePtr;
    typedef std::vector<SamplePtr> Samples;

    UsdGeomMesh         m_mesh;
    Samples             m_samples;
    TopologyVariance    m_topology_variance;
};

} // namespace usdi
