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
    usdiTrace("Camera::Camera(): %s\n", getPath());
}

Camera::~Camera()
{
    usdiTrace("Camera::~Camera(): %s\n", getPath());
}

UsdGeomCamera& Camera::getUSDType()
{
    return m_cam;
}

SchemaType Camera::getType() const
{
    return SchemaType::Camera;
}

bool Camera::readSample(CameraData& dst, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getImportConfig();

    {
        GfVec2f range;
        m_cam.GetClippingRangeAttr().Get(&range, t);
        dst.near_clipping_plane = range[0];
        dst.far_clipping_plane = range[1];
    }

    {
        static float Rad2Deg = 180.0f / float(M_PI);
        float focal_length;
        float focus_distance;
        float vertical_aperture;
        float horizontal_aperture;

        m_cam.GetFocalLengthAttr().Get(&focal_length, t);
        m_cam.GetFocusDistanceAttr().Get(&focus_distance, t);
        m_cam.GetVerticalApertureAttr().Get(&vertical_aperture, t);
        m_cam.GetHorizontalApertureAttr().Get(&horizontal_aperture, t);

        dst.field_of_view = 2.0f * atanf(vertical_aperture / (2.0f * focal_length)) * Rad2Deg;
        dst.aspect_ratio = float(horizontal_aperture / vertical_aperture);
        dst.focus_distance = focus_distance;
        dst.focal_length = focal_length;
        dst.aperture = vertical_aperture;
    }

    return true;
}

bool Camera::writeSample(const CameraData& src, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getExportConfig();

    {
        auto range = GfVec2f(src.near_clipping_plane, src.far_clipping_plane);
        m_cam.GetClippingRangeAttr().Set(range, t);
    }

    {
        const float Deg2Rad = float(M_PI) / 180.0f;
        float focal_length = src.focal_length;
        if (focal_length = 0.0f) {
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
