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
    return false;
}

bool Camera::writeSample(const CameraData& src, Time t_)
{
    return false;
}

} // namespace usdi
