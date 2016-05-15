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

bool MeshSample::read(UsdGeomMesh& mesh, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    bool ret = mesh.GetPointsAttr().Get(&points, t);
    mesh.GetNormalsAttr().Get(&normals, t);
    mesh.GetFaceVertexCountsAttr().Get(&counts, t);
    mesh.GetFaceVertexIndicesAttr().Get(&indices, t);
    return ret;
}

bool MeshSample::write(UsdGeomMesh& mesh, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    bool ret = mesh.GetPointsAttr().Set(points, t);
    mesh.GetNormalsAttr().Set(normals, t);
    mesh.GetFaceVertexCountsAttr().Set(counts, t);
    mesh.GetFaceVertexIndicesAttr().Set(indices, t);
    return ret;
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

    usdiTrace("Mesh::Mesh(): %s\n", getPath());
}

Mesh::~Mesh()
{
    usdiTrace("Mesh::~Mesh() %s\n", getPath());
}

UsdGeomMesh& Mesh::getUSDType()
{
    return m_mesh;
}

SchemaType Mesh::getType() const
{
    return SchemaType::Mesh;
}

void Mesh::getSummary(MeshSummary& dst) const
{
    dst.topology_variance = m_topology_variance;
    // todo
}

bool Mesh::readSample(MeshData& dst, Time t)
{
    const auto& conf = getImportConfig();

    MeshSample sample;
    if (!sample.read(m_mesh, t)) {
        return false;
    }

    dst.num_points = sample.points.size();
    dst.num_counts = sample.counts.size();
    dst.num_indices = sample.indices.size();
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
    if (dst.counts) {
        memcpy(dst.counts, &sample.counts[0], sizeof(int) * dst.num_counts);
    }
    if (dst.indices) {
        memcpy(dst.indices, &sample.indices[0], sizeof(int) * dst.num_indices);
    }

    if (conf.triangulate) {
        dst.num_indices_triangulated = GetTriangulatedIndexCount(sample.counts);
        if (dst.indices_triangulated) {
            TriangulateIndices(dst.indices_triangulated, sample.counts, &sample.indices, conf.swap_faces);
        }
    }

    return true;
}

bool Mesh::writeSample(const MeshData& src, Time t)
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

    if (src.indices) {
        sample.indices.assign(src.indices, src.indices + src.num_indices);
    }

    if (src.counts) {
        sample.counts.assign(src.counts, src.counts + src.num_counts);
    }
    else {
        // assume all faces are triangles
        size_t ntriangles = src.num_indices / 3;
        sample.counts.assign(ntriangles, 3);
    }

    return sample.write(m_mesh, t);
}

} // namespace usdi
