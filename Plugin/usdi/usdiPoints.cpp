#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiPoints.h"

namespace usdi {

void PointsSample::clear()
{
    points.clear();
    velocities.clear();
}


Points::Points(Context *ctx, Schema *parent, const UsdGeomPoints& points)
    : super(ctx, parent, points)
    , m_points(points)
{
    usdiLogTrace("Points::Points(): %s\n", getPath());
}

Points::Points(Context *ctx, Schema *parent, const char *name)
    : super(ctx, parent, name, "Points")
    , m_points(m_prim)
{
    usdiLogTrace("Points::Points(): %s\n", getPath());
}

Points::~Points()
{
    usdiLogTrace("Points::~Points(): %s\n", getPath());
}

UsdGeomPoints& Points::getUSDSchema()
{
    return m_points;
}

void Points::updateSummary() const
{
    m_summary_needs_update = false;
    m_summary.has_velocities = m_points.GetVelocitiesAttr().HasValue();
    //
}


const PointsSummary& Points::getSummary() const
{
    if (m_summary_needs_update) {
        updateSummary();
    }
    return m_summary;
}

bool Points::readSample(PointsData& dst, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();

    auto& sample = m_sample;
    if (m_prev_time != t_) {
        m_prev_time = t_;
        sample.clear();

        m_points.GetPointsAttr().Get(&sample.points, t);
        m_points.GetVelocitiesAttr().Get(&sample.velocities, t);
    }


    dst.num_points = sample.points.size();
    if (dst.points) {
        memcpy(dst.points, &sample.points[0], sizeof(float3) * dst.num_points);
        if (conf.swap_handedness) {
            for (uint i = 0; i < dst.num_points; ++i) {
                dst.points[i].x *= -1.0f;
            }
        }
    }
    if (dst.velocities) {
        if (sample.velocities.size() != dst.num_points) {
            usdiLogWarning("Points::readSample(): num points != size of velocity!!\n");
        }
        else {
            memcpy(dst.velocities, &sample.velocities[0], sizeof(float3) * dst.num_points);
            if (conf.swap_handedness) {
                for (uint i = 0; i < dst.num_points; ++i) {
                    dst.velocities[i].x *= -1.0f;
                }
            }
        }
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
