#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiPolyMesh.h"

namespace usdi {


PolyMeshSample::PolyMeshSample(PolyMesh *parent)
    : m_parent(parent)
{

}


PolyMesh::PolyMesh(Schema *parent, const UsdGeomMesh& mesh)
    : super(parent)
    , m_mesh(mesh)
    , m_topology_variance(TopologyVariance::Constant)
{
    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Heterogenous;
    }
}

PolyMeshSample* PolyMesh::readSample(UsdTimeCode t)
{
    auto* ret = new PolyMeshSample(this);
    m_mesh.GetPointsAttr().Get(&ret->m_points, t);
    m_mesh.GetNormalsAttr().Get(&ret->m_normals, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&ret->m_face_vertex_counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&ret->m_face_vertex_indices, t);
    return ret;
}

} // namespace usdi
