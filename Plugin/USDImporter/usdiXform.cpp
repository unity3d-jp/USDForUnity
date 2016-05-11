#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"

namespace usdi {

XformSample::XformSample(Xform *xf)
    : m_xf()
{
}


Xform::Xform(Schema *parent, const UsdGeomXformable& xf)
    : super(parent)
    , m_xf(xf)
{
}

XformSample* Xform::getSample(Time t_)
{
    auto t = (const UsdTimeCode&)t_.time;
    return new XformSample(this);
}

} // namespace usdi
