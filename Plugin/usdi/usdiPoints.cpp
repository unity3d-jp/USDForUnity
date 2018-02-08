#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPoints.h"
#include "usdiUtils.h"
#include "usdiContext.h"
#include "usdiAttribute.h"

namespace usdi {


RegisterSchemaHandler(Points)

Points::Points(Context *ctx, Schema *parent, const UsdPrim& prim)
    : super(ctx, parent, prim)
    , m_points(prim)
{
    usdiLogTrace("Points::Points(): %s\n", getPath());
    if (!m_points) { usdiLogError("Points::Points(): m_points is invalid\n"); }

    m_attr_ids64 = findAttribute("ids", AttributeType::Int64Array);
    if (m_attr_ids64) {
        m_attr_ids32 = m_attr_ids64->findOrCreateConverter(AttributeType::IntArray);
    }
}

Points::Points(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_points(m_prim)
{
    usdiLogTrace("Points::Points(): %s\n", getPath());
    if (!m_points) { usdiLogError("Points::Points(): m_points is invalid\n"); }
}

Points::~Points()
{
    usdiLogTrace("Points::~Points(): %s\n", getPath());
}

const PointsSummary& Points::getSummary() const
{
    if (m_summary_needs_update) {
        getTimeRange(m_summary.start, m_summary.end);
        m_summary.has_velocities = m_points.GetVelocitiesAttr().HasValue();

        m_summary_needs_update = false;
    }
    return m_summary;
}

void Points::updateSample(Time t_)
{
    super::updateSample(t_);
    if (m_update_flag.bits == 0) { return; }
    if (m_update_flag.variant_set_changed) { m_summary_needs_update = true; }

    auto t = UsdTimeCode(t_);
    const auto& conf = getImportSettings();

    // swap front sample
    if (!m_front_sample) {
        m_front_sample = &m_sample[0];
    }
    auto& sample = *m_front_sample;

    m_points.GetPointsAttr().Get(&sample.points, t);
    m_points.GetVelocitiesAttr().Get(&sample.velocities, t);
    m_points.GetWidthsAttr().Get(&sample.widths, t);

    if (conf.swap_handedness) {
        InvertX((float3*)sample.points.data(), sample.points.size());
        InvertX((float3*)sample.velocities.data(), sample.velocities.size());
    }
    if (conf.scale_factor != 1.0f) {
        Scale((float3*)sample.points.data(), conf.scale_factor, sample.points.size());
        Scale((float3*)sample.velocities.data(), conf.scale_factor, sample.velocities.size());
        Scale(sample.widths.data(), conf.scale_factor, sample.widths.size());
    }

    if (m_attr_ids64) {
        m_attr_ids64->getImmediate(&sample.ids64, t_);
    }
    if (m_attr_ids32) {
        m_attr_ids32->getImmediate(&sample.ids32, t_);
    }
}

bool Points::readSample(PointsData& dst, Time t, bool copy)
{
    if (t != m_time_prev) { updateSample(t); }

    if (!m_front_sample) { return false; }
    const auto& sample = *m_front_sample;

    dst.num_points = (uint)sample.points.size();
    if (copy) {
        if (dst.points && !sample.points.empty()) {
            memcpy(dst.points, sample.points.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.velocities && !sample.velocities.empty()) {
            memcpy(dst.velocities, sample.velocities.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.widths && !sample.widths.empty()) {
            memcpy(dst.widths, sample.widths.data(), sizeof(float) * dst.num_points);
        }
        if (dst.ids64 && !sample.ids64.empty()) {
            memcpy(dst.widths, sample.widths.data(), sizeof(int64_t) * dst.num_points);
        }
        if (dst.ids32 && !sample.ids32.empty()) {
            memcpy(dst.widths, sample.widths.data(), sizeof(int32_t) * dst.num_points);
        }
    }
    else {
        dst.points = (float3*)sample.points.cdata();
        dst.velocities = (float3*)sample.velocities.cdata();
        dst.widths = (float*)sample.widths.cdata();
        dst.ids64 = (int64_t*)sample.ids64.cdata();
        dst.ids32 = (int32_t*)sample.ids32.cdata();
    }

    return dst.num_points > 0;
}

bool Points::writeSample(const PointsData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportSettings();

    PointsSample sample;

    if (src.points) {
        sample.points.assign((GfVec3f*)src.points, (GfVec3f*)src.points + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.points) {
                v[0] *= -1.0f;
            }
        }
    }

    if (src.velocities) {
        sample.velocities.assign((GfVec3f*)src.velocities, (GfVec3f*)src.velocities + src.num_points);
        if (conf.swap_handedness) {
            for (auto& v : sample.velocities) {
                v[0] *= -1.0f;
            }
        }
    }

    bool  ret = m_points.GetPointsAttr().Set(sample.points, t);
    if (src.velocities) {
        m_points.GetVelocitiesAttr().Set(sample.velocities, t);
    }
    if (src.widths) {
        m_points.GetWidthsAttr().Set(sample.widths, t);
    }

#define CreateAttributeIfNeeded(VName, ...) if(!VName) { VName=createAttribute(__VA_ARGS__); }
    if (src.ids32) {
        sample.ids32.assign(src.ids32, src.ids32 + src.num_points);

        CreateAttributeIfNeeded(m_attr_ids32, "ids", AttributeType::Int, AttributeType::Int64);
        m_attr_ids32->setImmediate(&sample.ids32, t_);
    }
    else if (src.ids64) {
        sample.ids64.assign(src.ids64, src.ids64 + src.num_points);

        CreateAttributeIfNeeded(m_attr_ids64, "ids", AttributeType::Int64);
        m_attr_ids64->setImmediate(&sample.ids64, t_);
    }
#undef CreateAttributeIfNeeded

    m_summary_needs_update = true;
    return ret;
}

int Points::eachSample(const SampleCallback & cb)
{
    static const char *attr_names[] = {
        "points",
        "velocities",
        "widths",
        "ids",
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

    PointsData data;
    for (const auto& t : times) {
        readSample(data, t.first, false);
        cb(data, t.first);
    }
    return (int)times.size();
}

} // namespace usdi
