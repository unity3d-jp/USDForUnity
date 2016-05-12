#pragma once

namespace usdi {


struct MeshSample
{
public:
    MeshSample();
    void read(UsdGeomMesh& mesh, Time t);
    void write(UsdGeomMesh& mesh, Time t);

    VtArray<GfVec3f> points;
    VtArray<GfVec3f> normals;
    VtArray<int>     face_vertex_counts;
    VtArray<int>     face_vertex_indices;
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh);
    ~Mesh() override;

    UsdGeomMesh&    getUSDType() override;
    SchemaType      getType() const override;

    void            readSample(MeshData& dst, Time t);
    void            writeSample(const MeshData& src, Time t);

private:
    typedef MeshSample Sample;
    typedef std::unique_ptr<Sample> SamplePtr;
    typedef std::vector<SamplePtr> Samples;

    UsdGeomMesh         m_mesh;
    Samples             m_samples;
    TopologyVariance    m_topology_variance;
};

} // namespace usdi
