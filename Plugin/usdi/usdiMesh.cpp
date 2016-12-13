#include "pch.h"
#include "usdiInternal.h"
#include "usdiAttribute.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiMesh.h"
#include "usdiUtils.h"
#include "usdiContext.h"
#include "usdiContext.i"

namespace usdi {

const int usdiMaxVertices = 64998;

static inline void CountIndices(const VtArray<int> &counts, VtArray<int>& offsets, int& num_indices, int& num_indices_triangulated)
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


void SubmeshSample::clear()
{
    points.clear();
    normals.clear();
    uvs.clear();
    indices.clear();
    bounds_min = {}, bounds_max = {};
    center = {}, extents = {};
}

void MeshSample::clear()
{
    points.clear();
    velocities.clear();
    normals.clear();
    tangents.clear();
    uvs.clear();
    counts.clear();
    offsets.clear();
    indices.clear();
    indices_triangulated.clear();

    bone_weights.clear();
    bone_indices.clear();
    bones.clear();
    bones_.clear();
    root_bone = TfToken();
    weights4.clear();
    weights8.clear();
    max_bone_weights = 0;

    bounds_min = {}, bounds_max = {};
    center = {}, extents = {};
}


#define usdiUVAttrName              "primvars:uv"
#define usdiUVAttrName2             "uv"
#define usdiTangentAttrName         "tangents"
#define usdiBoneWeightsAttrName     "boneWeights"
#define usdiBoneIndicesAttrName     "boneIndices"
#define usdiBindPosesAttrName       "bindposes"
#define usdiBonesAttrName           "bones"
#define usdiRootBoneAttrName        "rootBone"
#define usdiMaxBoneWeightAttrName   "maxBoneWeights"

RegisterSchemaHandler(Mesh)

Mesh::Mesh(Context *ctx, Schema *parent, const UsdPrim& prim)
    : super(ctx, parent, prim)
    , m_mesh(prim)
{
    usdiLogTrace("Mesh::Mesh(): %s\n", getPath());
    if (!m_mesh) { usdiLogError("Mesh::Mesh(): m_mesh is invalid\n"); }

    m_attr_uv = findAttribute(usdiUVAttrName, AttributeType::Float2Array);
    if (!m_attr_uv) { m_attr_uv = findAttribute(usdiUVAttrName2, AttributeType::Float2Array); }
    m_attr_tangents = findAttribute(usdiTangentAttrName, AttributeType::Float4Array);

    // bone & weight attributes
    m_attr_bone_weights = findAttribute(usdiBoneWeightsAttrName, AttributeType::FloatArray);
    m_attr_bone_indices = findAttribute(usdiBoneIndicesAttrName, AttributeType::IntArray);
    m_attr_bindposes = findAttribute(usdiBindPosesAttrName, AttributeType::Float4x4Array);
    m_attr_bones = findAttribute(usdiBonesAttrName, AttributeType::TokenArray);
    m_attr_root_bone = findAttribute(usdiRootBoneAttrName, AttributeType::Token);
    m_attr_max_bone_weights = findAttribute(usdiMaxBoneWeightAttrName, AttributeType::Int);
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
        m_summary.has_uvs = m_attr_uv && m_attr_uv->hasValue();
        m_summary.has_tangents =
            m_summary.has_normals && m_summary.has_uvs && (
                (m_attr_tangents && m_attr_tangents->hasValue()) ||
                settings.tangent_calculation != TangentCalculationType::Never);
        m_summary.has_velocities = m_mesh.GetVelocitiesAttr().HasValue();

        if (m_attr_bones && m_attr_bones->hasValue()) {
            VtArray<TfToken> bones;
            m_attr_bones->getImmediate(&bones, usdiDefaultTime());
            m_summary.num_bones = (uint)bones.size();
        }
        if (m_attr_max_bone_weights && m_attr_max_bone_weights->hasValue()) {
            m_attr_max_bone_weights->getImmediate(&m_summary.max_bone_weights, usdiDefaultTime());
        }

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

    // swap front sample
    if (!m_front_sample) {
        m_front_sample = &m_sample[0];
        m_front_submesh = &m_submeshes[0];
    }
    else if(conf.double_buffering) {
        if (m_front_sample == &m_sample[0]) {
            m_front_sample = &m_sample[1];
            m_front_submesh = &m_submeshes[1];
        }
        else {
            m_front_sample = &m_sample[0];
            m_front_submesh = &m_submeshes[0];
        }
    }
    auto& sample = *m_front_sample;
    auto& splits = *m_front_submesh;

    m_mesh.GetPointsAttr().Get(&sample.points, t);
    m_mesh.GetVelocitiesAttr().Get(&sample.velocities, t);
    m_mesh.GetFaceVertexCountsAttr().Get(&sample.counts, t);
    m_mesh.GetFaceVertexIndicesAttr().Get(&sample.indices, t);
    if (m_attr_uv) {
        m_attr_uv->getImmediate(&sample.uvs, t_);
    }

    // apply swap_handedness and scale
    if (conf.swap_handedness) {
        InvertX((float3*)sample.points.data(), sample.points.size());
        InvertX((float3*)sample.velocities.data(), sample.velocities.size());
    }
    if (conf.scale != 1.0f) {
        Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
    }

    // normals
    bool needs_calculate_normals = conf.normal_calculation == NormalCalculationType::Always;
    if (!needs_calculate_normals) {
        if (m_mesh.GetNormalsAttr().Get(&sample.normals, t)) {
            if (conf.swap_handedness) {
                InvertX((float3*)sample.normals.data(), sample.normals.size());
            }
        }
        else {
            if (conf.normal_calculation == NormalCalculationType::WhenMissing) {
                needs_calculate_normals = true;
            }
            else {
                // no normal data is present and no recalculation is required.
                // just allocate empty normal array.
                sample.normals.resize(sample.points.size());
                memset(sample.normals.data(), 0, sizeof(float3)*sample.normals.size());
            }
        }
    }

    // tangents
    bool needs_calculate_tangents = conf.tangent_calculation == NormalCalculationType::Always;
    if (!needs_calculate_tangents) {
        if (m_attr_tangents && m_attr_tangents->getImmediate(&sample.tangents, t_)) {
            if (conf.swap_handedness) {
                InvertX((float4*)sample.tangents.data(), sample.tangents.size());
            }
        }
        else {
            if (m_attr_uv && conf.tangent_calculation == TangentCalculationType::WhenMissing) {
                needs_calculate_tangents = true;
            }
            else {
                // nothing to do here
            }
        }
    }

    // indices
    bool needs_calculate_indices =
        m_num_indices_triangulated == 0 ||
        getSummary().topology_variance == TopologyVariance::Heterogenous ||
        m_update_flag.import_config_updated || m_update_flag.variant_set_changed;
    bool needs_copy_indices =
        sample.indices_triangulated.size() != m_sample[0].indices_triangulated.size() ||
        m_update_flag_prev.variant_set_changed;
    if (needs_calculate_indices) {
        CountIndices(sample.counts, sample.offsets, m_num_indices, m_num_indices_triangulated);
        if (conf.triangulate || needs_calculate_normals) {
            sample.indices_triangulated.resize(m_num_indices_triangulated);
            TriangulateIndices(sample.indices_triangulated.data(), sample.counts, &sample.indices, conf.swap_faces);
        }
    }
    else if (needs_copy_indices) {
        sample.indices_triangulated = m_sample[0].indices_triangulated;
        sample.offsets = m_sample[0].offsets;
    }

    // calculate normals if needed
    if (needs_calculate_normals) {
        sample.normals.resize(sample.points.size());
        CalculateNormals((float3*)sample.normals.data(), (const float3*)sample.points.cdata(), sample.indices_triangulated.cdata(),
            sample.points.size(), sample.indices_triangulated.size());
    }

    // calculate tangents if needed
    if (needs_calculate_tangents) {
        sample.tangents.resize(sample.points.size());
        CalculateTangents((float4*)sample.tangents.data(),
            (const float3*)sample.points.cdata(), (const float3*)sample.normals.cdata(), (const float2*)sample.uvs.cdata(),
            sample.counts.cdata(), sample.offsets.cdata(), sample.indices.cdata(),
            sample.points.size(), sample.counts.size());
    }

    // bone & weights
    // assume these are constant (get values only once)
    if (m_attr_bone_weights && m_attr_bone_indices && (sample.weights4.empty() || sample.weights8.empty())) {
        if (m_attr_max_bone_weights) {
            m_attr_max_bone_weights->getImmediate(&sample.max_bone_weights, t_);
            if (sample.max_bone_weights == 0) {
                goto END_WEIGHTS;
            }
        }
        if (sample.max_bone_weights != 4 && sample.max_bone_weights != 8) {
            usdiLogError("sample.max_bone_weights != 4 && sample.max_bone_weights != 8\n");
            goto END_WEIGHTS;
        }

        m_attr_bone_weights->getImmediate(&sample.bone_weights, t_);
        m_attr_bone_indices->getImmediate(&sample.bone_indices, t_);
        if (sample.bone_weights.size() != sample.bone_indices.size()) {
            usdiLogError("sample.bone_weights.size() != sample.bone_indices.size()\n");
            goto END_WEIGHTS;
        }

        // weight array & index array -> weight4 array or weight8 array
        if (sample.max_bone_weights == 4) {
            const size_t nweights = 4;
            const size_t npoints = sample.bone_weights.size() / nweights;
            sample.weights4.resize(npoints);
            for (size_t ip = 0; ip < npoints; ++ip) {
                size_t ip4 = ip * nweights;
                auto& w = sample.weights4[ip];
                for (size_t iw = 0; iw < nweights; ++iw) {
                    w.weight[iw] = sample.bone_weights[ip4 + iw];
                    w.indices[iw] = sample.bone_indices[ip4 + iw];
                }
            }
        }
        else if (sample.max_bone_weights == 8) {
            const size_t nweights = 8;
            const size_t npoints = sample.bone_weights.size() / nweights;
            sample.weights8.resize(npoints);
            for (size_t ip = 0; ip < npoints; ++ip) {
                size_t ip8 = ip * nweights;
                auto& w = sample.weights8[ip];
                for (size_t iw = 0; iw < nweights; ++iw) {
                    w.weight[iw] = sample.bone_weights[ip8 + iw];
                    w.indices[iw] = sample.bone_indices[ip8 + iw];
                }
            }
        }

    END_WEIGHTS:;
    }
    if (m_attr_bones && sample.bones.empty()) {
        m_attr_bones->getImmediate(&sample.bones, t_);
        sample.bones_.resize(sample.bones.size());
        for (size_t i = 0; i < sample.bones.size(); ++i) {
            sample.bones_[i] = sample.bones[i].GetText();
        }
    }
    if (m_attr_root_bone && sample.root_bone.IsEmpty()) {
        m_attr_root_bone->getImmediate(&sample.root_bone, t_);
    }
    if (m_attr_bindposes && sample.bindposes.empty()) {
        m_attr_bindposes->getImmediate(&sample.bindposes, t_);
    }

    // bounds
    ComputeBounds((const float3*)sample.points.cdata(), sample.points.size(), sample.bounds_min, sample.bounds_max);
    sample.center = (sample.bounds_min + sample.bounds_max) * 0.5f;
    sample.extents = (sample.bounds_max - sample.bounds_min) * 0.5f;


    // mesh split

    bool points_are_expanded = sample.points.size() == m_num_indices;
    bool normals_are_expanded = sample.normals.size() == m_num_indices;
    bool tangents_are_expanded = sample.tangents.size() == m_num_indices;
    bool uvs_are_expanded = sample.uvs.size() == m_num_indices;
    bool weights_are_expanded = sample.weights4.size() == m_num_indices || sample.weights8.size() == m_num_indices;

    bool needs_split = false;
    if (conf.split_mesh) {
        needs_split = sample.points.size() > usdiMaxVertices || points_are_expanded || normals_are_expanded || uvs_are_expanded;
    }
    if (!needs_split) { return; }

    int num_splits = ceildiv(m_num_indices_triangulated, usdiMaxVertices);
    splits.resize(num_splits);

    for (int nth = 0; nth < num_splits; ++nth) {
        auto& sms = splits[nth];
        int ibegin = usdiMaxVertices * nth;
        int iend = std::min<int>(usdiMaxVertices * (nth+1), m_num_indices_triangulated);
        int isize = iend - ibegin;

        {
            sms.indices.resize(isize);
            for (int i = 0; i < isize; ++i) { sms.indices[i] = i; }
        }
        CopyWithIndices(sms.points, sample.points, sample.indices_triangulated, ibegin, iend, !points_are_expanded);
        CopyWithIndices(sms.normals, sample.normals, sample.indices_triangulated, ibegin, iend, !normals_are_expanded);
        CopyWithIndices(sms.tangents, sample.tangents, sample.indices_triangulated, ibegin, iend, !tangents_are_expanded);
        CopyWithIndices(sms.uvs, sample.uvs, sample.indices_triangulated, ibegin, iend, !uvs_are_expanded);
        if (!sample.weights4.empty()) {
            CopyWithIndices(sms.weights4, sample.weights4, sample.indices_triangulated, ibegin, iend, !weights_are_expanded);
        }
        else if (!sample.weights8.empty()) {
            CopyWithIndices(sms.weights8, sample.weights8, sample.indices_triangulated, ibegin, iend, !weights_are_expanded);
        }

        ComputeBounds((float3*)sms.points.cdata(), sms.points.size(), sms.bounds_min, sms.bounds_max);
        sms.center = (sms.bounds_min + sms.bounds_max) * 0.5f;
        sms.extents = sms.bounds_max - sms.bounds_min;
    }
}

bool Mesh::readSample(MeshData& dst, Time t, bool copy)
{
    if (t != m_time_prev) { updateSample(t); }

    if (!m_front_sample) { return false; }

    const auto& sample = *m_front_sample;
    const auto& splits = *m_front_submesh;

    dst.num_points = (uint)sample.points.size();
    dst.num_counts = (uint)sample.counts.size();
    dst.num_indices = (uint)sample.indices.size();
    dst.num_indices_triangulated = m_num_indices_triangulated;
    dst.num_submeshes = (uint)splits.size();
    dst.center = sample.center;
    dst.extents = sample.extents;

    dst.max_bone_weights = sample.max_bone_weights;
    dst.bones = (char**)&sample.bones_[0];
    dst.root_bone = (char*)sample.root_bone.GetText();
    dst.num_bones = (int)sample.bones_.size();

    if (copy) {
        if (dst.points && !sample.points.empty()) {
            memcpy(dst.points, sample.points.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.velocities && !sample.velocities.empty()) {
            memcpy(dst.velocities, sample.velocities.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.normals && !sample.normals.empty()) {
            memcpy(dst.normals, sample.normals.cdata(), sizeof(float3) * dst.num_points);
        }
        if (dst.tangents && !sample.tangents.empty()) {
            memcpy(dst.tangents, sample.tangents.cdata(), sizeof(float4) * dst.num_points);
        }
        if (dst.uvs && !sample.uvs.empty()) {
            memcpy(dst.uvs, sample.uvs.cdata(), sizeof(float2) * dst.num_points);
        }
        if (dst.counts && !sample.counts.empty()) {
            memcpy(dst.counts, sample.counts.cdata(), sizeof(int) * dst.num_counts);
        }
        if (dst.indices && !sample.indices.empty()) {
            memcpy(dst.indices, sample.indices.cdata(), sizeof(int) * dst.num_indices);
        }
        if (dst.indices_triangulated && !sample.indices_triangulated.empty()) {
            memcpy(dst.indices_triangulated, sample.indices_triangulated.cdata(), sizeof(int) * m_num_indices_triangulated);
        }

        if (dst.weights4 && !sample.weights4.empty()) {
            memcpy(dst.weights4, sample.weights4.cdata(), sizeof(Weights4) * dst.num_points);
        }
        if (dst.weights8 && !sample.weights8.empty()) {
            memcpy(dst.weights8, sample.weights8.cdata(), sizeof(Weights8) * dst.num_points);
        }
        if (dst.bindposes && !sample.bindposes.empty()) {
            memcpy(dst.bindposes, sample.bindposes.cdata(), sizeof(float4x4) * dst.num_bones);
        }

        if (dst.submeshes) {
            for (size_t i = 0; i < dst.num_submeshes; ++i) {
                const auto& ssrc = splits[i];
                auto& sdst = dst.submeshes[i];
                sdst.num_points = (uint)ssrc.points.size();
                sdst.center = ssrc.center;
                sdst.extents = ssrc.extents;

                if (sdst.indices && !ssrc.indices.empty()) {
                    memcpy(sdst.indices, ssrc.indices.cdata(), sizeof(int) * sdst.num_points);
                }
                if (sdst.points && !ssrc.points.empty()) {
                    memcpy(sdst.points, ssrc.points.cdata(), sizeof(float3) * sdst.num_points);
                }
                if (sdst.normals && !ssrc.normals.empty()) {
                    memcpy(sdst.normals, ssrc.normals.cdata(), sizeof(float3) * sdst.num_points);
                }
                if (sdst.tangents && !ssrc.tangents.empty()) {
                    memcpy(sdst.tangents, ssrc.tangents.cdata(), sizeof(float4) * sdst.num_points);
                }
                if (sdst.uvs && !ssrc.uvs.empty()) {
                    memcpy(sdst.uvs, ssrc.uvs.cdata(), sizeof(float2) * sdst.num_points);
                }

                if (sdst.weights4 && !ssrc.weights4.empty()) {
                    memcpy(sdst.weights4, ssrc.weights4.cdata(), sizeof(Weights4) * dst.num_points);
                }
                if (sdst.weights8 && !ssrc.weights8.empty()) {
                    memcpy(sdst.weights8, ssrc.weights8.cdata(), sizeof(Weights8) * dst.num_points);
                }
            }
        }
    }
    else {
        dst.points = (float3*)sample.points.cdata();
        dst.velocities = (float3*)sample.velocities.cdata();
        dst.normals = (float3*)sample.normals.cdata();
        dst.tangents = (float4*)sample.tangents.cdata();
        dst.uvs = (float2*)sample.uvs.cdata();
        dst.counts = (int*)sample.counts.cdata();
        dst.indices = (int*)sample.indices.cdata();
        dst.indices_triangulated = (int*)sample.indices_triangulated.cdata();

        if (!sample.weights4.empty()) {
            dst.weights4 = (Weights4*)sample.weights4.cdata();
        }
        else if (!sample.weights8.empty()) {
            dst.weights8 = (Weights8*)sample.weights8.cdata();
        }
        dst.bindposes = (float4x4*)sample.bindposes.cdata();

        if (dst.submeshes) {
            for (size_t i = 0; i < dst.num_submeshes; ++i) {
                const auto& ssrc = splits[i];
                auto& sdst = dst.submeshes[i];
                sdst.num_points = (uint)ssrc.points.size();
                if (sdst.indices && !ssrc.indices.empty()) {
                    sdst.indices = (int*)ssrc.indices.cdata();
                }
                if (sdst.points && !ssrc.points.empty()) {
                    sdst.points = (float3*)ssrc.points.cdata();
                }
                if (sdst.normals && !ssrc.normals.empty()) {
                    sdst.normals = (float3*)ssrc.normals.cdata();
                }
                if (sdst.uvs && !ssrc.uvs.empty()) {
                    sdst.uvs = (float2*)ssrc.uvs.cdata();
                }
            }
        }
    }

    return dst.num_points > 0;
}

bool Mesh::writeSample(const MeshData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportSettings();

    MeshSample& sample = m_sample[0];

#define CreateAttributeIfNeeded(VName, AName, Type) if(!VName) { VName=createAttribute(AName, Type); }

    bool  ret = false;
    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.points.data(), sample.points.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        }
        ret = m_mesh.GetPointsAttr().Set(sample.points, t);
    }

    if (src.velocities) {
        sample.points.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.velocities.data(), sample.velocities.size());
        }
        if (conf.scale != 1.0f) {
            Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
        }
        m_mesh.GetVelocitiesAttr().Set(sample.velocities, t);
    }

    if (src.normals) {
        sample.normals.assign((GfVec3f*)src.normals, (GfVec3f*)src.normals + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float3*)sample.normals.data(), sample.normals.size());
        }
        m_mesh.GetNormalsAttr().Set(sample.normals, t);
    }

    if (src.indices) {
        if (src.counts) {
            sample.counts.assign(src.counts, src.counts + src.num_counts);
        }
        else {
            // assume all faces are triangles
            size_t ntriangles = src.num_indices / 3;
            sample.counts.assign(ntriangles, 3);
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

            sample.indices.resize(src.num_indices);
            copy_with_swap(sample.indices, src.indices, sample.counts);
        }
        else {
            sample.indices.assign(src.indices, src.indices + src.num_indices);
        }
        m_mesh.GetFaceVertexCountsAttr().Set(sample.counts, t);
        m_mesh.GetFaceVertexIndicesAttr().Set(sample.indices, t);
    }

    if (src.uvs) {
        sample.uvs.assign((GfVec2f*)src.uvs, (GfVec2f*)src.uvs + src.num_points);

        CreateAttributeIfNeeded(m_attr_uv, usdiUVAttrName, AttributeType::Float2Array);
        m_attr_uv->setImmediate(&sample.uvs, t_);
    }

    if (src.tangents) {
        sample.tangents.assign((GfVec4f*)src.tangents, (GfVec4f*)src.tangents + src.num_points);
        if (conf.swap_handedness) {
            InvertX((float4*)sample.tangents.data(), sample.tangents.size());
        }

        CreateAttributeIfNeeded(m_attr_tangents, usdiTangentAttrName, AttributeType::Float4Array);
        m_attr_tangents->setImmediate(&sample.tangents, t_);
    }


    // bone & weight attributes

    if (src.weights4 && src.max_bone_weights == 4) {
        sample.max_bone_weights = src.max_bone_weights;
        sample.bone_weights.resize(src.num_points * 4);
        sample.bone_indices.resize(src.num_points * 4);
        for (uint pi = 0; pi < src.num_points; ++pi) {
            int pi4 = pi * 4;
            for (int i = 0; i < 4; ++i) {
                sample.bone_weights[pi4 + i] = src.weights4[pi].weight[i];
                sample.bone_indices[pi4 + i] = src.weights4[pi].indices[i];
            }
        }

        CreateAttributeIfNeeded(m_attr_bone_weights, usdiBoneWeightsAttrName, AttributeType::FloatArray);
        CreateAttributeIfNeeded(m_attr_bone_indices, usdiBoneIndicesAttrName, AttributeType::IntArray);
        CreateAttributeIfNeeded(m_attr_max_bone_weights, usdiMaxBoneWeightAttrName, AttributeType::Int);
        m_attr_bone_weights->setImmediate(&sample.bone_weights, t_);
        m_attr_bone_indices->setImmediate(&sample.bone_indices, t_);
        m_attr_max_bone_weights->setImmediate(&sample.max_bone_weights, t_);
    }
    if (src.bindposes) {
        sample.bindposes.assign((GfMatrix4f*)src.bindposes, (GfMatrix4f*)src.bindposes + src.num_bones);
        CreateAttributeIfNeeded(m_attr_bindposes, usdiBindPosesAttrName, AttributeType::Float4x4Array);
        m_attr_bindposes->setImmediate(&sample.bindposes, t_);
    }
    if (src.bones) {
        sample.bones.resize(src.num_bones);
        for (uint bi = 0; bi < src.num_bones; ++bi) {
            sample.bones[bi] = TfToken(src.bones[bi]);
        }

        CreateAttributeIfNeeded(m_attr_bones, usdiBonesAttrName, AttributeType::TokenArray);
        m_attr_bones->setImmediate(&sample.bones, t_);
    }
    if (src.root_bone) {
        sample.root_bone = TfToken(src.root_bone);

        CreateAttributeIfNeeded(m_attr_root_bone, usdiRootBoneAttrName, AttributeType::Token);
        m_attr_root_bone->setImmediate(&sample.root_bone, t_);
    }

#undef CreateAttributeIfNeeded

    m_summary_needs_update = true;
    return ret;
}

void Mesh::assignRootBone(MeshData& dst, const char *v)
{
    MeshSample& sample = m_sample[0];
    sample.root_bone = TfToken(v);
    dst.root_bone = (char*)sample.root_bone.GetText();
}

void Mesh::assignBones(MeshData& dst, const char **v, int n)
{
    MeshSample& sample = m_sample[0];
    sample.bones.resize(n);
    sample.bones_.resize(n);
    for (int i = 0; i < n; ++i) {
        sample.bones[i] = TfToken(v[i]);
        sample.bones_[i] = sample.bones[i].GetText();
    }
    dst.bones = (char**)sample.bones_.data();
    dst.num_bones = n;
}


} // namespace usdi
