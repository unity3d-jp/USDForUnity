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

void MeshSample::clear()
{
    points.clear();
    velocities.clear();
    normals.clear();
    counts.clear();
    indices.clear();
}



Mesh::Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh)
    : super(ctx, parent, UsdGeomXformable(mesh))
    , m_mesh(mesh)
{
    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_topology_variance = TopologyVariance::Heterogenous;
    }

    usdiTrace("Mesh::Mesh(): %s\n", getPath());
}

Mesh::Mesh(Context *ctx, Schema *parent, const char *name)
    : super(ctx, parent, name, "Mesh")
    , m_mesh(m_prim)
{
    usdiTrace("Mesh::Mesh(): %s\n", getPath());
}

Mesh::~Mesh()
{
    usdiTrace("Mesh::~Mesh() %s\n", getPath());
}

UsdGeomMesh& Mesh::getUSDSchema()
{
    return m_mesh;
}

void Mesh::getSummary(MeshSummary& dst) const
{
    dst.topology_variance = m_topology_variance;
    // todo
}

bool Mesh::readSample(MeshData& dst, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getImportConfig();

    MeshSample& sample = m_rsample;
    m_mesh.GetPointsAttr().Get(&sample.points, t);
    m_mesh.GetVelocitiesAttr().Get(&sample.velocities, t);
    m_mesh.GetNormalsAttr().Get(&sample.normals, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&sample.counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&sample.indices, t);


    dst.num_points = sample.points.size();
    dst.num_counts = sample.counts.size();
    dst.num_indices = sample.indices.size();
    if (dst.points && !sample.points.empty()) {
        memcpy(dst.points, &sample.points[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.points[i].x *= -1.0f;
            }
        }
        if (conf.scale != 1.0f) {
            float s = conf.scale;
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.points[i] *= s;
            }
        }
    }
    if (dst.velocities && !sample.velocities.empty()) {
        memcpy(dst.velocities, &sample.velocities[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.velocities[i].x *= -1.0f;
            }
        }
        if (conf.scale != 1.0f) {
            float s = conf.scale;
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.velocities[i] *= s;
            }
        }
    }
    if (dst.normals && !sample.normals.empty()) {
        memcpy(dst.normals, &sample.normals[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.normals[i].x *= -1.0f;
            }
        }
    }
    if (dst.counts && !sample.counts.empty()) {
        memcpy(dst.counts, &sample.counts[0], sizeof(int) * dst.num_counts);
    }
    if (dst.indices && !sample.indices.empty()) {
        memcpy(dst.indices, &sample.indices[0], sizeof(int) * dst.num_indices);
    }

    if (conf.triangulate) {
        dst.num_indices_triangulated = GetTriangulatedIndexCount(sample.counts);
        if (dst.indices_triangulated) {
            TriangulateIndices(dst.indices_triangulated, sample.counts, &sample.indices, conf.swap_faces);
        }
    }

    sample.clear();
    return dst.num_points > 0;
}

bool Mesh::writeSample(const MeshData& src, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getExportConfig();

    MeshSample& sample = m_wsample;

    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.points) {
                v[0] *= -1.0f;
            }
        }
        if (conf.scale != 1.0f) {
            float s = conf.scale;
            for (auto& v : sample.points) {
                (float3&)v *= s;
            }
        }
    }

    if (src.velocities) {
        sample.points.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.velocities) {
                v[0] *= -1.0f;
            }
        }
        if (conf.scale != 1.0f) {
            float s = conf.scale;
            for (auto& v : sample.velocities) {
                (float3&)v *= s;
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

    if (src.counts) {
        sample.counts.assign(src.counts, src.counts + src.num_counts);
    }
    else {
        // assume all faces are triangles
        size_t ntriangles = src.num_indices / 3;
        sample.counts.assign(ntriangles, 3);
    }

    if (src.indices) {
        if (conf.swap_faces) {
            auto copy_with_swap = [](VtArray<int>& dst_indices, const int *src_indices, const VtArray<int>& counts) {
                int i = 0;
                for (int ngon : counts) {
                    for (int ni = 0; ni < ngon; ++ni) {
                        int ini = ngon - ni - 1;
                        dst_indices[i + ni] = src_indices[i + ini];
                    }
                    i += ngon;
                }
            };

            sample.indices.resize(src.num_indices);
            copy_with_swap(sample.indices, src.indices, sample.counts);
        }
        else {
            sample.indices.assign(src.indices, src.indices + src.num_indices);
        }
    }


    bool  ret = m_mesh.GetPointsAttr().Set(sample.points, t);
    m_mesh.GetVelocitiesAttr().Set(sample.velocities, t);
    m_mesh.GetNormalsAttr().Set(sample.normals, t);
    m_mesh.GetFaceVertexCountsAttr().Set(sample.counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Set(sample.indices, t);
    sample.clear();
    return ret;
}

} // namespace usdi
