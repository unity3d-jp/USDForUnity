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

    XformSample* getSample(Time t);

private:
    const UsdGeomXformable& m_xf;
};

} // namespace usdi
