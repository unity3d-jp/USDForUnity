#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"

namespace usdi {


Xform::Xform(Schema *parent, const UsdGeomXformable& xf)
    : super(parent)
    , m_xf(xf)
{
    usdiLog("constructed\n");
}

Xform::~Xform()
{
    usdiLog("destructed\n");
}

UsdGeomXformable& Xform::getUSDType()
{
    return m_xf;
}

void Xform::readSample(Time t, XformData& dst)
{

}

void Xform::writeSample(Time t, const XformData& src)
{

}

} // namespace usdi
