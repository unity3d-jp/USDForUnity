#pragma once

namespace usdi {

class CameraSample
{
public:
};


class Camera : public Xform
{
typedef Xform super;
public:
    Camera(Context *ctx, Schema *parent, const UsdGeomCamera& xf);
    ~Camera() override;

    UsdGeomCamera&      getUSDType() override;
    SchemaType          getType() const override;

    bool                readSample(CameraData& dst, Time t);
    bool                writeSample(const CameraData& src, Time t);

private:
    UsdGeomCamera       m_cam;
};

} // namespace usdi
