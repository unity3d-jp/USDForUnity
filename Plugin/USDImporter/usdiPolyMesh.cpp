#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPolyMesh.h"

namespace usdi {


MeshSample::MeshSample()
{
}
void MeshSample::read(UsdGeomMesh& mesh, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    mesh.GetPointsAttr().Get(&points, t);
    mesh.GetNormalsAttr().Get(&normals, t);
    mesh.GetFaceVertexCountsAttr().Get(&face_vertex_counts, t);
    mesh.GetFaceVertexIndicesAttr().Get(&face_vertex_indices, t);
}

void MeshSample::write(UsdGeomMesh& mesh, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    mesh.GetPointsAttr().Set(points, t);
    mesh.GetNormalsAttr().Set(normals, t);
    mesh.GetFaceVertexCountsAttr().Set(face_vertex_counts, t);
    mesh.GetFaceVertexIndicesAttr().Set(face_vertex_indices, t);
}


Mesh::Mesh(Schema *parent, const UsdGeomMesh& mesh)
    : super(parent, UsdGeomXformable(mesh))
    , m_mesh(mesh)
    , m_topology_variance(TopologyVariance::Constant)
{
    usdiLog("constructed\n");

    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Heterogenous;
    }
}

Mesh::~Mesh()
{
    usdiLog("destructed\n");
}

UsdGeomMesh& Mesh::getUSDType()
{
    return m_mesh;
}


void Mesh::readSample(MeshData& dst, Time t)
{
    MeshSample sample;
    sample.read(m_mesh, t);

    dst.num_points = sample.points.size();
    dst.num_face_vertex_counts = sample.face_vertex_counts.size();
    dst.num_face_vertex_indices = sample.face_vertex_indices.size();
    if (dst.points) {
        memcpy(dst.points, &sample.points[0], sizeof(float3) * dst.num_points);
    }
    if (dst.normals) {
        memcpy(dst.normals, &sample.normals[0], sizeof(float3) * dst.num_points);
    }
    if (dst.face_vertex_counts) {
        memcpy(dst.face_vertex_counts, &sample.face_vertex_counts[0], sizeof(int) * dst.num_face_vertex_counts);
    }
    if (dst.face_vertex_indices) {
        memcpy(dst.face_vertex_indices, &sample.face_vertex_indices[0], sizeof(int) * dst.num_face_vertex_indices);
    }
}

void Mesh::writeSample(const MeshData& src, Time t)
{
    MeshSample sample;
    sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
    if (src.face_vertex_indices) {
        sample.face_vertex_indices.assign(src.face_vertex_indices, src.face_vertex_indices + src.num_face_vertex_indices);
    }

    if (src.face_vertex_counts) {
        sample.face_vertex_counts.assign(src.face_vertex_counts, src.face_vertex_counts + src.num_face_vertex_counts);
    }
    else {
        // assume all faces are triangles
        size_t ntriangles = src.num_face_vertex_indices / 3;
        sample.face_vertex_counts.assign(ntriangles, 3);
    }

    sample.write(m_mesh, t);
}

} // namespace usdi
