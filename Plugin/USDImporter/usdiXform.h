#pragma once

namespace usdi {

class XformSample : public Sample
{
public:
    XformSample(Xform *xf);

public:
    Xform *m_xf;
};


class Xform : public Schema
{
typedef Schema super;
public:
    Xform(Schema *parent, const UsdGeomXformable& xf);

    UsdGeomXformable&   getUSDType() override;

    XformSample*        getSample(Time t);

private:
    UsdGeomXformable    m_xf;
};

} // namespace usdi
