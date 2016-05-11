#pragma once

namespace usdi {

class XformSample : public Sample
{

};


class Xform : public Schema
{
typedef Schema super;
public:
    Xform(Schema *parent, const UsdGeomXformable& xf);

private:
    const UsdGeomXformable& m_xf;
};

} // namespace usdi
