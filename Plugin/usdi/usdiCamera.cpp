#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiCamera.h"

namespace usdi {


Camera::Camera(Context *ctx, Schema *parent, const UsdGeomCamera& cam)
    : super(ctx, parent, cam)
    , m_cam(cam)
{
    usdiLogTrace("Camera::Camera(): %s\n", getPath());
    getTimeRange(m_summary.start, m_summary.end);
}

Camera::Camera(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_cam(m_prim)
{
    usdiLogTrace("Camera::Camera(): %s\n", getPath());
}

Camera::~Camera()
{
    usdiLogTrace("Camera::~Camera(): %s\n", getPath());
}

UsdGeomCamera& Camera::getUSDSchema()
{
    return m_cam;
}


const usdi::CameraSummary& Camera::getSummary() const
{
    return m_summary;
}

void Camera::updateSample(Time t_)
{
    super::updateSample(t_);
    if (!needsUpdate()) { return; }

    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();
    auto& sample = m_sample;

    {
        GfVec2f range;
        m_cam.GetClippingRangeAttr().Get(&range, t);
        sample.near_clipping_plane = range[0];
        sample.far_clipping_plane = range[1];
    }
    {
        float focal_length;
        float focus_distance;
        float vertical_aperture;
        float horizontal_aperture;

        m_cam.GetFocalLengthAttr().Get(&focal_length, t);
        m_cam.GetFocusDistanceAttr().Get(&focus_distance, t);
        m_cam.GetVerticalApertureAttr().Get(&vertical_aperture, t);
        m_cam.GetHorizontalApertureAttr().Get(&horizontal_aperture, t);

        sample.field_of_view = 2.0f * atanf(vertical_aperture / (2.0f * focal_length)) * Rad2Deg;
        sample.aspect_ratio = float(horizontal_aperture / vertical_aperture);
        sample.focus_distance = focus_distance;
        sample.focal_length = focal_length;
        sample.aperture = vertical_aperture;
    }
}

bool Camera::readSample(CameraData& dst, Time t)
{
    if (t != m_time_prev) { updateSample(t); }

    dst = m_sample;
    return true;
}

bool Camera::writeSample(const CameraData& src, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportConfig();

    {
        auto range = GfVec2f(src.near_clipping_plane, src.far_clipping_plane);
        m_cam.GetClippingRangeAttr().Set(range, t);
    }

    {
        float focal_length = src.focal_length;
        if (focal_length == 0.0f) {
            focal_length = src.aperture / std::tan(src.field_of_view * Deg2Rad / 2.0f) / 2.0f;
        }

        m_cam.GetFocalLengthAttr().Set(focal_length, t);
        m_cam.GetFocusDistanceAttr().Set(src.focus_distance, t);
        m_cam.GetVerticalApertureAttr().Set(src.aperture, t);
        m_cam.GetHorizontalApertureAttr().Set(src.aperture * src.aspect_ratio, t);
    }

    return true;
}

} // namespace usdi
