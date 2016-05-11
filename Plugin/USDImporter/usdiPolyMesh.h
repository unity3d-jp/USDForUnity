#pragma once

namespace usdi {


enum class TopologyVariance
{
    Constant, // both vertices and topologies are constant
    Homogenous, // vertices are not constant (= animated). topologies are constant.
    Heterogenous, // both vertices and topologies are not constant
};

class PolyMeshSample : public Sample
{
public:
    PolyMeshSample(PolyMesh *parent);

public:
    PolyMesh        *m_parent;
    VtArray<GfVec3f> m_points;
    VtArray<GfVec3f> m_normals;
    VtArray<int>     m_face_vertex_counts;
    VtArray<int>     m_face_vertex_indices;
};


class PolyMesh : public Schema
{
typedef Schema super;
public:
    PolyMesh(Schema *parent, const UsdGeomMesh& mesh);

    PolyMeshSample* readSample(UsdTimeCode t);

private:
    const UsdGeomMesh& m_mesh;
    TopologyVariance m_topology_variance;
};

} // namespace usdi
