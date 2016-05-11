#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"

namespace usdi {

Xform::Xform(Schema *parent, const UsdGeomXformable& xf)
    : super(parent)
    , m_xf(xf)
{
}

} // namespace usdi
