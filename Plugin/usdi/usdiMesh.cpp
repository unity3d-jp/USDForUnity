#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"

namespace usdi {


template<class CountArray>
static inline uint GetTriangulatedIndexCount(const CountArray &counts)
{
    uint r = 0;
    size_t num_faces = counts.size();
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        r += (counts[fi] - 2) * 3;
    }
    return r;
}

template<class CountArray, class IndexArray>
static inline void TriangulateIndices(int *triangulated, const CountArray &counts, const IndexArray *indices, bool swap_face)
{
    const int i1 = swap_face ? 2 : 1;
    const int i2 = swap_face ? 1 : 2;
    size_t num_faces = counts.size();

    int n = 0;
    int i = 0;
    if (indices) {
        for (size_t fi = 0; fi < num_faces; ++fi) {
            int ngon = counts[fi];
            for (int ni = 0; ni < ngon - 2; ++ni) {
                triangulated[i + 0] = (*indices)[n + 0];
                triangulated[i + 1] = (*indices)[n + ni + i1];
                triangulated[i + 2] = (*indices)[n + ni + i2];
                i += 3;
            }
            n += ngon;
        }
    }
    else {
        for (size_t fi = 0; fi < num_faces; ++fi) {
            int ngon = counts[fi];
            for (int ni = 0; ni < ngon - 2; ++ni) {
                triangulated[i + 0] = n + 0;
                triangulated[i + 1] = n + ni + i1;
                triangulated[i + 2] = n + ni + i2;
                i += 3;
            }
            n += ngon;
        }
    }
}



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


Mesh::Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh)
    : super(ctx, parent, UsdGeomXformable(mesh))
    , m_mesh(mesh)
    , m_topology_variance(TopologyVariance::Constant)
{
    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Heterogenous;
    }

    usdiLog("constructed %s\n", getPath());
}

Mesh::~Mesh()
{
    usdiLog("destructed %s\n", getPath());
}

UsdGeomMesh& Mesh::getUSDType()
{
    return m_mesh;
}

SchemaType Mesh::getType() const
{
    return SchemaType::Mesh;
}

void Mesh::readSample(MeshData& dst, Time t)
{
    const auto& conf = getImportConfig();

    MeshSample sample;
    sample.read(m_mesh, t);

    dst.num_points = sample.points.size();
    dst.num_face_vertex_counts = sample.face_vertex_counts.size();
    dst.num_face_vertex_indices = sample.face_vertex_indices.size();
    if (dst.points) {
        memcpy(dst.points, &sample.points[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.points[i].x *= -1.0f;
            }
        }
    }
    if (dst.normals) {
        memcpy(dst.normals, &sample.normals[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.normals[i].x *= -1.0f;
            }
        }
    }
    if (dst.face_vertex_counts) {
        memcpy(dst.face_vertex_counts, &sample.face_vertex_counts[0], sizeof(int) * dst.num_face_vertex_counts);
    }
    if (dst.face_vertex_indices) {
        memcpy(dst.face_vertex_indices, &sample.face_vertex_indices[0], sizeof(int) * dst.num_face_vertex_indices);
    }

    if (conf.triangulate) {
        dst.num_face_vertex_indices_triangulated = GetTriangulatedIndexCount(sample.face_vertex_counts);
        if (dst.face_vertex_indices_triangulated) {
            TriangulateIndices(dst.face_vertex_indices_triangulated, sample.face_vertex_counts, &sample.face_vertex_indices, conf.swap_faces);
        }
    }
}

void Mesh::writeSample(const MeshData& src, Time t)
{
    const auto& conf = getExportConfig();

    MeshSample sample;

    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.points) {
                v[0] *= -1.0f;
            }
        }
    }

    if (src.normals) {
        sample.normals.assign((GfVec3f*)src.normals, (GfVec3f*)src.normals + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.normals) {
                v[0] *= -1.0f;
            }
        }
    }

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
