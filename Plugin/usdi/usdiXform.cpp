#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiContext.h"

namespace usdi {


Xform::Xform(Context *ctx, Schema *parent, const UsdGeomXformable& xf)
    : super(ctx, parent, xf)
    , m_xf(xf)
{
    usdiTrace("Xform::Xform(): %s\n", getPath());
}

Xform::Xform(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_xf(m_prim)
{
    usdiTrace("Xform::Xform(): %s\n", getPath());
}

Xform::~Xform()
{
    usdiTrace("Xform::~Xform(): %s\n", getPath());
}

UsdGeomXformable& Xform::getUSDSchema()
{
    return m_xf;
}

bool Xform::readSample(XformData& dst, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getImportConfig();
    dst.position = { 0.0f, 0.0f, 0.0f };
    dst.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    dst.scale = { 1.0f, 1.0f, 1.0f };

    if (m_read_ops.empty()) {
        bool reset_stack = false;
        m_read_ops = m_xf.GetOrderedXformOps(&reset_stack);
    }

    bool ret = false;
    for (auto& op : m_read_ops) {
        switch (op.GetOpType()) {
        case UsdGeomXformOp::TypeTranslate:
        {
            if (op.GetAs((GfVec3f*)&dst.position, t)) { ret = true; }
            break;
        }
        case UsdGeomXformOp::TypeOrient:
        {
            if (op.GetAs((GfQuatf*)&dst.rotation, t)) { ret = true; }
            break;
        }
        case UsdGeomXformOp::TypeScale:
        {
            if (op.GetAs((GfVec3f*)&dst.scale, t)) { ret = true; }
            break;
        }

        case UsdGeomXformOp::TypeRotateZXY:
        {
            // todo
            break;
        }
        case UsdGeomXformOp::TypeTransform:
        {
            // todo
            break;
        }
        }
    }
    return ret;
}

bool Xform::writeSample(const XformData& src_, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getImportConfig();

    if (m_write_ops.empty()) {
        m_write_ops.push_back(m_xf.AddTranslateOp(UsdGeomXformOp::PrecisionFloat));
        m_write_ops.push_back(m_xf.AddOrientOp(UsdGeomXformOp::PrecisionFloat));
        m_write_ops.push_back(m_xf.AddScaleOp(UsdGeomXformOp::PrecisionFloat));
    }
    {
        XformData src = src_;
        if (conf.swap_handedness) {
            src.position.x *= -1.0f;
            src.rotation = { -src.rotation.x, -src.rotation.z, src.rotation.y, -src.rotation.w };
        }
        m_write_ops[0].Set((const GfVec3f&)src.position, t);
        m_write_ops[1].Set((const GfQuatf&)src.rotation, t);
        m_write_ops[2].Set((const GfVec3f&)src.scale, t);
    }
    return true;
}

} // namespace usdi
