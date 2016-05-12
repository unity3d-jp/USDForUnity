#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPolyMesh.h"

namespace usdi {



MeshSample::MeshSample(Mesh *parent)
    : m_parent(parent)
{
}

void MeshSample::readData(MeshData& dst)
{
    dst.num_points = m_points.size();
    dst.num_face_vertex_counts = m_face_vertex_counts.size();
    dst.num_face_vertex_indices = m_face_vertex_indices.size();
    if (dst.points) {
        memcpy(dst.points, &m_points[0], sizeof(float3) * dst.num_points);
    }
    if (dst.normals) {
        memcpy(dst.normals, &m_normals[0], sizeof(float3) * dst.num_points);
    }
    if (dst.face_vertex_counts) {
        memcpy(dst.face_vertex_counts, &m_face_vertex_counts[0], sizeof(int) * dst.num_face_vertex_counts);
    }
    if (dst.face_vertex_indices) {
        memcpy(dst.face_vertex_indices, &m_face_vertex_indices[0], sizeof(int) * dst.num_face_vertex_indices);
    }
}

void MeshSample::writeData(const MeshData& src)
{
    if (!src.points) {
        usdiLog("src.points is null");
        return;
    }

    m_points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
    if (src.face_vertex_indices) {
        m_face_vertex_indices.assign(src.face_vertex_indices, src.face_vertex_indices + src.num_face_vertex_indices);
    }

    if (src.face_vertex_counts) {
        m_face_vertex_counts.assign(src.face_vertex_counts, src.face_vertex_counts + src.num_face_vertex_counts);
    }
    else {
        // assume all faces are triangles
        size_t ntriangles = src.num_face_vertex_indices / 3;
        m_face_vertex_counts.assign(ntriangles, 3);
    }
}


Mesh::Mesh(Schema *parent, const UsdGeomMesh& mesh)
    : super(parent, UsdGeomXformable(mesh))
    , m_mesh(mesh)
    , m_topology_variance(TopologyVariance::Constant)
{
    usdiLog("");

    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Heterogenous;
    }
}

UsdGeomMesh& Mesh::getUSDType()
{
    return m_mesh;
}

MeshSample* Mesh::getSample(Time t_)
{
    auto t = (const UsdTimeCode&)t_.time;
    auto* ret = new MeshSample(this);
    m_mesh.GetPointsAttr().Get(&ret->m_points, t);
    m_mesh.GetNormalsAttr().Get(&ret->m_normals, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&ret->m_face_vertex_counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&ret->m_face_vertex_indices, t);
    return ret;
}

} // namespace usdi
