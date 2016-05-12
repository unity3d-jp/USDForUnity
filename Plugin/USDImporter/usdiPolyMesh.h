#pragma once

namespace usdi {


class MeshSample : public Sample
{
public:
    MeshSample(Mesh *parent);

    void readData(MeshData& dst);
    void writeData(const MeshData& src);

public:
    Mesh            *m_parent;
    VtArray<GfVec3f> m_points;
    VtArray<GfVec3f> m_normals;
    VtArray<int>     m_face_vertex_counts;
    VtArray<int>     m_face_vertex_indices;
};


class Mesh : public Xform
{
typedef Xform super;
public:
    Mesh(Schema *parent, const UsdGeomMesh& mesh);

    UsdGeomMesh&    getUSDType() override;

    MeshSample*     getSample(Time t);

private:
    UsdGeomMesh         m_mesh;
    TopologyVariance    m_topology_variance;
};

} // namespace usdi
