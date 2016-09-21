#pragma once

namespace usdi {

class Camera : public Xform
{
typedef Xform super;
public:
    Camera(Context *ctx, Schema *parent, const UsdGeomCamera& xf);
    Camera(Context *ctx, Schema *parent, const char *name);
    ~Camera() override;

    UsdGeomCamera&      getUSDSchema() override;
    void                updateSample(Time t) override;

    bool                readSample(CameraData& dst, Time t);
    bool                writeSample(const CameraData& src, Time t);

private:
    UsdGeomCamera       m_cam;
    CameraData          m_sample;
};

} // namespace usdi
