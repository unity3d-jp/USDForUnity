#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiUtils.h"
#include "usdiContext.h"


namespace usdi {

static inline void CountIndices(
    const VtArray<int> &counts,
    VtArray<int>& offsets,
    int& num_indices,
    int& num_indices_triangulated)
{
    int reti = 0, rett = 0;
    size_t num_faces = counts.size();
    offsets.resize(num_faces);
    for (size_t fi = 0; fi < num_faces; ++fi)
    {
        auto f = counts[fi];
        offsets[fi] = reti;
        reti += f;
        rett += (f - 2) * 3;
    }
    num_indices = reti;
    num_indices_triangulated = rett;
}


template<class T, class IndexArray>
inline void CopyWithIndices(T *dst, const T *src, const IndexArray& indices)
{
    if (!dst || !src) { return; }
    size_t size = indices.size();
    for (size_t i = 0; i < (int)size; ++i) {
        dst[i] = src[indices[i]];
    }
}

template<class T>
inline void Remap(RawVector<T>& dst, const T *src, const RawVector<int>& indices)
{
    dst.resize_discard(indices.size());
    CopyWithIndices(dst.data(), src, indices);
}

static inline void assign(Weights4& dst, const Weights8& src)
{
    // maybe need to sort weights..
    memcpy(dst.weight, src.weight, sizeof(float) * 4);
    memcpy(dst.indices, src.indices, sizeof(int) * 4);

    // normalize
    float scale = 1.0f / (src.weight[0] + src.weight[1] + src.weight[2] + src.weight[3]);
    for (float& w : dst.weight) {
        w *= scale;
    }
}

static inline void assign(Weights8& dst, const Weights4& src)
{
    memcpy(dst.weight, src.weight, sizeof(float) * 4);
    memcpy(dst.indices, src.indices, sizeof(int) * 4);
    memset(dst.weight + 4, 0, sizeof(float) * 4);
    memset(dst.indices + 4, 0, sizeof(int) * 4);
}


RegisterSchemaHandler(Mesh)

Mesh::Mesh(Context *ctx, Schema *parent, const UsdPrim& prim)
    : super(ctx, parent, prim)
    , m_mesh(prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    if (!m_mesh) { usdiLogError("Mesh::Mesh(): m_mesh is invalid\n"); }

    m_attr_colors = findAttribute(usdiColorAttrName, AttributeType::Float4Array);
    m_attr_uv0 = findAttribute(usdiUVAttrName, AttributeType::Float2Array);
}

Mesh::Mesh(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_mesh(m_prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    if (!m_mesh) { usdiLogError("Mesh::Mesh(): m_mesh is invalid\n"); }
}

Mesh::~Mesh()
{
    usdiLogTrace("Mesh::~Mesh(): %s\n", getPath());
}

const MeshSummary& Mesh::getSummary() const
{
    if (m_summary_needs_update) {
        auto& settings = getImportSettings();

        getTimeRange(m_summary.start, m_summary.end);
        m_summary.has_normals =
            m_mesh.GetNormalsAttr().HasValue() ||
            settings.normal_calculation != NormalCalculationType::Never;
        m_summary.has_colors = m_attr_colors && m_attr_colors->hasValue();
        m_summary.has_uv0 = m_attr_uv0 && m_attr_uv0->hasValue();
        m_summary.has_tangents = m_summary.has_normals && m_summary.has_uv0 && settings.tangent_calculation != TangentCalculationType::Never;
        m_summary.has_velocities = m_mesh.GetVelocitiesAttr().HasValue();

        if (m_mesh.GetPointsAttr().ValueMightBeTimeVarying()) {
            m_summary.topology_variance = TopologyVariance::Homogenous;
        }
        if (m_mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying() ||
            m_mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying()) {
            m_summary.topology_variance = TopologyVariance::Heterogenous;
        }

        m_summary_needs_update = false;
    }
    return m_summary;
}


void Mesh::updateSample(Time t_)
{
    super::updateSample(t_);
    if (m_update_flag.bits == 0) { return; }
    if (m_update_flag.variant_set_changed) { m_summary_needs_update = true; }


    auto t = UsdTimeCode(t_);
    const auto& conf = getImportSettings();

    auto& sample = m_sample;

    m_mesh.GetPointsAttr().Get(&sample.points_sp, t);
    m_mesh.GetVelocitiesAttr().Get(&sample.velocities_sp, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&sample.counts_sp, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&sample.indices_sp, t);
    if (m_attr_colors) {
        m_attr_colors->getImmediate(&sample.colors_sp, t_);
    }
    if (m_attr_uv0) {
        m_attr_uv0->getImmediate(&sample.uv0_sp, t_);
    }

    // apply swap_handedness and scale
    if (conf.swap_handedness) {
        SwapHandedness((float3*)sample.points_sp.data(), (int)sample.points_sp.size());
        SwapHandedness((float3*)sample.velocities_sp.data(), (int)sample.velocities_sp.size());
    }
    if (conf.scale_factor != 1.0f) {
        ApplyScale((float3*)sample.points_sp.data(), (int)sample.points_sp.size(), conf.scale_factor);
        ApplyScale((float3*)sample.velocities_sp.data(), (int)sample.velocities_sp.size(), conf.scale_factor);
    }

    // normals
    bool gen_normals = conf.normal_calculation == NormalCalculationType::Always;
    if (!gen_normals) {
        if (m_mesh.GetNormalsAttr().Get(&sample.normals_sp, t)) {
            if (conf.swap_handedness) {
                SwapHandedness((float3*)sample.normals_sp.data(), (int)sample.normals_sp.size());
            }
        }
        else {
            if (conf.normal_calculation == NormalCalculationType::WhenMissing) {
                gen_normals = true;
            }
            else {
                // no normal data is present and no recalculation is required.
                // just allocate empty normal array.
                sample.normals_sp.resize(sample.points_sp.size());
                memset(sample.normals_sp.data(), 0, sizeof(float3)*sample.normals_sp.size());
            }
        }
    }

    // indices
    bool update_indices =
        m_num_indices_triangulated == 0 ||
        getSummary().topology_variance == TopologyVariance::Heterogenous ||
        m_update_flag.import_settings_updated || m_update_flag.variant_set_changed;
    bool copy_indices =
        sample.indices_triangulated.size() != sample.indices_triangulated.size() ||
        m_update_flag_prev.variant_set_changed;
    if (update_indices) {
        //CountIndices(sample.counts_sp, sample.offsets, m_num_indices, m_num_indices_triangulated);
        //if (conf.triangulate || gen_normals) {
        //    sample.indices_triangulated.resize(m_num_indices_triangulated);
        //    TriangulateIndices(sample.indices_triangulated, sample.counts_sp, &sample.indices_sp, conf.swap_faces);
        //}
    }
    else if (copy_indices) {
        sample.indices_triangulated = sample.indices_triangulated;
        sample.offsets = sample.offsets;
    }

    // normals
    if (gen_normals) {
        sample.normals.resize(sample.points_sp.size());
        GenerateNormals(sample.normals.data(),
            sample.points.data(), sample.indices_triangulated.data(), (int)sample.normals.size(), (int)sample.indices_triangulated.size() / 3);
    }

    // tangents
    bool gen_tangents = conf.tangent_calculation != TangentCalculationType::Never;
    if (gen_tangents) {
        sample.tangents.resize(sample.points_sp.size());
        //GenerateTangents(ToIArray(sample.tangents),
        //    ToIArray(sample.points_sp), ToIArray(sample.normals_sp), ToIArray(sample.uv0_sp),
        //    ToIArray(sample.counts_sp), ToIArray(sample.offsets), ToIArray(sample.indices_sp));
    }

    // bounds
    MinMax(sample.bounds_min, sample.bounds_max, (const float3*)sample.points_sp.cdata(), (int)sample.points_sp.size());
    sample.center = (sample.bounds_min + sample.bounds_max) * 0.5f;
    sample.extents = (sample.bounds_max - sample.bounds_min) * 0.5f;


    // submesh

    const int max_vertices = conf.split_unit;
//    {
//
//        if (sample.indices_flattened_triangulated.size() != sample.indices_triangulated.size() || update_indices)
//        {
//            sample.indices_flattened_triangulated.resize(m_num_indices_triangulated);
//            TriangulateIndices(sample.indices_flattened_triangulated, sample.counts_sp, nullptr, conf.swap_faces);
//        }
//
//        // split meshes and flatten vertices
//        for (int nth = 0; nth < m_num_current_submeshes; ++nth) {
//            auto& sms = submeshes[nth];
//            int ibegin = max_vertices * nth;
//            int iend = std::min<int>(max_vertices * (nth + 1), m_num_indices_triangulated);
//            int isize = iend - ibegin;
//
//            sms.indices.resize(isize);
//            for (int i = 0; i < isize; ++i) { sms.indices[i] = i; }
//
//#define Sel(C) (C ? sample.indices_flattened_triangulated.cdata() : sample.indices_triangulated.cdata())
//            CopyWithIndices(sms.points.data(), sample.points_sp.cdata(), Sel(flattened.points), ibegin, iend);
//            CopyWithIndices(sms.normals.data(), sample.normals_sp.cdata(), Sel(flattened.normals), ibegin, iend);
//            CopyWithIndices(sms.colors.data(), sample.colors_sp.cdata(), Sel(flattened.colors), ibegin, iend);
//            CopyWithIndices(sms.uvs.data(), sample.uv0_sp.cdata(), Sel(flattened.uv0), ibegin, iend);
//            CopyWithIndices(sms.tangents.data(), sample.tangents.cdata(), Sel(flattened.tangents), ibegin, iend);
//            CopyWithIndices(sms.velocities.data(), sample.velocities_sp.cdata(), Sel(flattened.velocities), ibegin, iend);
//            if (!sample.weights4.empty()) {
//                CopyWithIndices(sms.weights4.data(), sample.weights4.cdata(), Sel(flattened.weights), ibegin, iend);
//            }
//            if (!sample.weights8.empty()) {
//                CopyWithIndices(sms.weights8.data(), sample.weights8.cdata(), Sel(flattened.weights), ibegin, iend);
//            }
//#undef Sel
//
//            MinMax(sms.bounds_min, sms.bounds_max, (float3*)sms.points.cdata(), (int)sms.points.size());
//            sms.center = (sms.bounds_min + sms.bounds_max) * 0.5f;
//            sms.extents = sms.bounds_max - sms.bounds_min;
//        }
//    }
}

bool Mesh::readSample(MeshData& dst)
{
    const auto& sample = m_sample;

    dst.vertex_count = (uint)sample.points_sp.size();
    dst.index_count = m_num_indices_triangulated;
    dst.center = sample.center;
    dst.extents = sample.extents;

    if (dst.points && !sample.points_sp.empty()) {
        memcpy(dst.points, sample.points_sp.cdata(), sizeof(float3) * dst.vertex_count);
    }
    if (dst.normals && !sample.normals_sp.empty()) {
        memcpy(dst.normals, sample.normals_sp.cdata(), sizeof(float3) * dst.vertex_count);
    }
    if (dst.colors && !sample.colors_sp.empty()) {
        memcpy(dst.colors, sample.colors_sp.cdata(), sizeof(float4) * dst.vertex_count);
    }
    if (dst.uv0 && !sample.uv0_sp.empty()) {
        memcpy(dst.uv0, sample.uv0_sp.cdata(), sizeof(float2) * dst.vertex_count);
    }
    if (dst.tangents && !sample.tangents.empty()) {
        memcpy(dst.tangents, sample.tangents.cdata(), sizeof(float4) * dst.vertex_count);
    }
    if (dst.velocities && !sample.velocities_sp.empty()) {
        memcpy(dst.velocities, sample.velocities_sp.cdata(), sizeof(float3) * dst.vertex_count);
    }
    if (dst.indices && !sample.indices_triangulated.empty()) {
        memcpy(dst.indices, sample.indices_triangulated.cdata(), sizeof(int) * m_num_indices_triangulated);
    }

    return dst.vertex_count > 0;
}

#define CreateAttributeIfNeeded(VName, ...) if(!VName) { VName=createAttribute(__VA_ARGS__); }

bool Mesh::writeSample(const MeshData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportSettings();

    MeshSample& sample = m_sample;


    bool  ret = false;
    if (src.points) {
        sample.points_sp.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.vertex_count);
        if (conf.swap_handedness) {
            SwapHandedness((float3*)sample.points_sp.data(), (int)sample.points_sp.size());
        }
        if (conf.scale_factor != 1.0f) {
            ApplyScale((float3*)sample.points_sp.data(), (int)sample.points_sp.size(), conf.scale_factor);
        }
        ret = m_mesh.GetPointsAttr().Set(sample.points_sp, t);
    }

    if (src.velocities) {
        sample.points_sp.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.vertex_count);
        if (conf.swap_handedness) {
            SwapHandedness((float3*)sample.velocities_sp.data(), (int)sample.velocities_sp.size());
        }
        if (conf.scale_factor != 1.0f) {
            ApplyScale((float3*)sample.velocities_sp.data(), (int)sample.velocities_sp.size(), conf.scale_factor);
        }
        m_mesh.GetVelocitiesAttr().Set(sample.velocities_sp, t);
    }

    if (src.normals) {
        sample.normals_sp.assign((GfVec3f*)src.normals, (GfVec3f*)src.normals + src.vertex_count);
        if (conf.swap_handedness) {
            SwapHandedness((float3*)sample.normals_sp.data(), (int)sample.normals_sp.size());
        }
        m_mesh.GetNormalsAttr().Set(sample.normals_sp, t);
    }

    if (src.indices) {
        if (src.faces) {
            sample.counts_sp.assign(src.faces, src.faces + src.face_count);
        }
        else {
            // assume all faces are triangles
            size_t ntriangles = src.index_count / 3;
            sample.counts_sp.assign(ntriangles, 3);
        }

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

            sample.indices_sp.resize(src.index_count);
            copy_with_swap(sample.indices_sp, src.indices, sample.counts_sp);
        }
        else {
            sample.indices_sp.assign(src.indices, src.indices + src.index_count);
        }
        m_mesh.GetFaceVertexCountsAttr().Set(sample.counts_sp, t);
        m_mesh.GetFaceVertexIndicesAttr().Set(sample.indices_sp, t);
    }

    if (src.colors) {
        sample.colors_sp.assign((GfVec4f*)src.colors, (GfVec4f*)src.colors + src.vertex_count);

        CreateAttributeIfNeeded(m_attr_colors, usdiColorAttrName, AttributeType::Float4Array);
        m_attr_colors->setImmediate(&sample.colors_sp, t_);
    }

    if (src.uv0) {
        sample.uv0_sp.assign((GfVec2f*)src.uv0, (GfVec2f*)src.uv0 + src.vertex_count);

        CreateAttributeIfNeeded(m_attr_uv0, usdiUVAttrName, AttributeType::Float2Array);
        m_attr_uv0->setImmediate(&sample.uv0_sp, t_);
    }

    m_summary_needs_update = true;
    return ret;
}

int Mesh::eachSample(const SampleCallback & cb)
{
    static const char *attr_names[] = {
        "faceVertexCounts",
        "faceVertexIndices",
        "normals",
        "points",
        "velocities",
        usdiUVAttrName,
        usdiUVAttrName2,
        usdiColorAttrName,
    };

    std::map<Time, int> times;
    for (auto *name : attr_names) {
        if (auto *attr = findAttribute(name)) {
            attr->eachTime([&times](Time t) {
                times[t] = 0;
            });
        }
    }
    if (times.empty()) {
        times[usdiDefaultTime()] = 0;
    }

    MeshData data;
    for (const auto& t : times) {
        updateSample(t.first);
        readSample(data);
        cb(data, t.first);
    }
    return (int)times.size();
}

#undef CreateAttributeIfNeeded

} // namespace usdi
