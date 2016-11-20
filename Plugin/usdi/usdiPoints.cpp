#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPoints.h"
#include "usdiUtils.h"

namespace usdi {


void PointsSample::clear()
{
    points.clear();
    velocities.clear();
}

const char *Points::UsdTypeName = "Points";

Points::Points(Context *ctx, Schema *parent, const UsdPrim& prim)
    : super(ctx, parent, prim)
    , m_points(prim)
{
    usdiLogTrace("Points::Points(): %s\n", getPath());
    if (!m_points) { usdiLogError("Points::Points(): m_points is invalid\n"); }
    getTimeRange(m_summary.start, m_summary.end);
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
    if (!needsUpdate()) { return; }

    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();

    // swap front sample
    if (!m_front_sample) {
        m_front_sample = &m_sample[0];
    }
    else if (conf.double_buffering) {
        if (m_front_sample == &m_sample[0]) {
            m_front_sample = &m_sample[1];
        }
        else {
            m_front_sample = &m_sample[0];
        }
    }
    auto& sample = *m_front_sample;

    m_points.GetPointsAttr().Get(&sample.points, t);
    m_points.GetVelocitiesAttr().Get(&sample.velocities, t);

    if (conf.swap_handedness) {
        InvertX((float3*)sample.points.data(), sample.points.size());
        InvertX((float3*)sample.velocities.data(), sample.velocities.size());
    }
    if (conf.scale != 1.0f) {
        Scale((float3*)sample.points.data(), conf.scale, sample.points.size());
        Scale((float3*)sample.velocities.data(), conf.scale, sample.velocities.size());
    }
}

void Points::invalidateSample()
{
    super::invalidateSample();
    m_summary_needs_update = true;
    for (auto& sample : m_sample) { sample.clear(); }
}

bool Points::readSample(PointsData& dst, Time t, bool copy)
{
    if (t != m_time_prev) { updateSample(t); }

    if (!m_front_sample) { return false; }
    const auto& sample = *m_front_sample;

    dst.num_points = (uint)sample.points.size();
    if (copy) {
        if (dst.points) {
            memcpy(dst.points, sample.points.data(), sizeof(float3) * dst.num_points);
        }
        if (dst.velocities) {
            if (sample.velocities.size() != dst.num_points) {
                usdiLogWarning("Points::readSample(): sample.points.size() != sample.velocities.size() !!\n");
            }
            else {
                memcpy(dst.velocities, sample.velocities.data(), sizeof(float3) * dst.num_points);
            }
        }
    }
    else {
        dst.points = (float3*)sample.points.data();
        dst.velocities = (float3*)sample.velocities.data();
    }

    return dst.num_points > 0;
}

bool Points::writeSample(const PointsData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportConfig();

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
    m_summary_needs_update = true;
    return ret;
}

} // namespace usdi
