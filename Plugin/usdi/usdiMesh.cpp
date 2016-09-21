#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiSIMD.h"

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
    uvs.clear();
    counts.clear();
    indices.clear();
    indices_triangulated.clear();
}


#define usdiUVAttrName "uv"

Mesh::Mesh(Context *ctx, Schema *parent, const UsdGeomMesh& mesh)
    : super(ctx, parent, UsdGeomXformable(mesh))
    , m_mesh(mesh)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    m_attr_uv = createAttribute(usdiUVAttrName, AttributeType::Float2Array);
}

Mesh::Mesh(Context *ctx, Schema *parent, const char *name)
    : super(ctx, parent, name, "Mesh")
    , m_mesh(m_prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    m_attr_uv = createAttribute(usdiUVAttrName, AttributeType::Float2Array);
}

Mesh::~Mesh()
{
    usdiLogTrace("Mesh::~Mesh(): %s\n", getPath());
}

UsdGeomMesh& Mesh::getUSDSchema()
{
    return m_mesh;
}

void Mesh::updateSummary() const
{
    m_summary_needs_update = false;

    m_summary.has_uvs = m_attr_uv && m_attr_uv->hasValue();
    m_summary.has_normals = m_mesh.GetNormalsAttr().HasValue();
    m_summary.has_velocities = m_mesh.GetVelocitiesAttr().HasValue();

    if (m_mesh.GetPointsAttr().ValueMightBeTimeVarying()) {
        m_summary.topology_variance = TopologyVariance::Homogenous;
    }
    if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying() ||
        m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
        m_summary.topology_variance = TopologyVariance::Heterogenous;
    }
}

const MeshSummary& Mesh::getSummary() const
{
    if (m_summary_needs_update) {
        updateSummary();
    }
    return m_summary;
}

void Mesh::updateSample(Time t_)
{
    if (m_prev_time == t_) {
        return;
    }
    super::updateSample(t_);

    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();
    MeshSample& sample = m_sample;

    m_mesh.GetPointsAttr().Get(&sample.points, t);
    m_mesh.GetVelocitiesAttr().Get(&sample.velocities, t);
    m_mesh.GetNormalsAttr().Get(&sample.normals, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&sample.counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&sample.indices, t);
    if (m_attr_uv) {
        m_attr_uv->get(&sample.uvs, t_);
    }
    if (m_num_indices_triangulated == 0 || getSummary().topology_variance == TopologyVariance::Heterogenous) {
        m_num_indices_triangulated = GetTriangulatedIndexCount(sample.counts);
        if (conf.triangulate) {
            sample.indices_triangulated.resize(m_num_indices_triangulated);
            TriangulateIndices(sample.indices_triangulated.data(), sample.counts, &sample.indices, conf.swap_faces);
        }
    }

    if (conf.swap_handedness) {
        InvertX((float3*)sample.points.data(), sample.points.size());
        InvertX((float3*)sample.velocities.data(), sample.velocities.size());
        InvertX((float3*)sample.normals.data(), sample.normals.size());
    }
    if (conf.scale != 1.0f) {
        Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
    }
}

bool Mesh::readSample(MeshData& dst, Time t, bool copy)
{
    updateSample(t);

    const MeshSample& sample = m_sample;
    dst.num_points = sample.points.size();
    dst.num_counts = sample.counts.size();
    dst.num_indices = sample.indices.size();
    dst.num_indices_triangulated = m_num_indices_triangulated;

    if (copy) {
        if (dst.points && !sample.points.empty()) {
            memcpy(dst.points, sample.points.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.velocities && !sample.velocities.empty()) {
            memcpy(dst.velocities, sample.velocities.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.normals && !sample.normals.empty()) {
            memcpy(dst.normals, sample.normals.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.uvs && !sample.uvs.empty()) {
            memcpy(dst.uvs, sample.uvs.data(), sizeof(float2) * dst.num_points);
        }
        if (dst.counts && !sample.counts.empty()) {
            memcpy(dst.counts, sample.counts.data(), sizeof(int) * dst.num_counts);
        }
        if (dst.indices && !sample.indices.empty()) {
            memcpy(dst.indices, sample.indices.data(), sizeof(int) * dst.num_indices);
        }
        if (dst.indices_triangulated && !sample.indices_triangulated.empty()) {
            memcpy(dst.indices_triangulated, sample.indices_triangulated.data(), sizeof(int) * m_num_indices_triangulated);
        }
    }
    else {
        dst.points = (float3*)sample.points.data();
        dst.velocities = (float3*)sample.velocities.data();
        dst.normals = (float3*)sample.normals.data();
        dst.uvs = (float2*)sample.uvs.data();
        dst.counts = (int*)sample.counts.data();
        dst.indices = (int*)sample.indices.data();
        dst.indices_triangulated = (int*)sample.indices_triangulated.data();
    }

    return dst.num_points > 0;
}

bool Mesh::writeSample(const MeshData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportConfig();

    MeshSample& sample = m_sample;

    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.points.data(), sample.points.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        }
    }

    if (src.velocities) {
        sample.points.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.velocities.data(), sample.velocities.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
        }
    }

    if (src.normals) {
        sample.normals.assign((GfVec3f*)src.normals, (GfVec3f*)src.normals + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.normals.data(), sample.normals.size());
        }
    }

    if (src.uvs) {
        sample.uvs.assign((GfVec2f*)src.uvs, (GfVec2f*)src.uvs + src.num_points);
    }

    if (src.counts) {
        sample.counts.assign(src.counts, src.counts + src.num_counts);
    }
    else if (src.indices) {
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
    if (src.velocities) {
        m_mesh.GetVelocitiesAttr().Set(sample.velocities, t);
    }
    if (src.normals) {
        m_mesh.GetNormalsAttr().Set(sample.normals, t);
    }
    if (src.indices) {
        m_mesh.GetFaceVertexCountsAttr().Set(sample.counts, t);
        m_mesh.GetFaceVertexIndicesAttr().Set(sample.indices, t);
    }
    if (src.uvs && m_attr_uv) {
        m_attr_uv->set(&sample.uvs, t_);
    }

    m_summary_needs_update = true;
    return ret;
}

} // namespace usdi
