#pragma once

namespace usdi {

class XformSample
{
public:
};


class Xform : public Schema
{
typedef Schema super;
public:
    Xform(Schema *parent, const UsdGeomXformable& xf);
    ~Xform() override;

    UsdGeomXformable&   getUSDType() override;

    void                readSample(Time t, XformData& dst);
    void                writeSample(Time t, const XformData& src);

private:
    UsdGeomXformable    m_xf;
};

} // namespace usdi
