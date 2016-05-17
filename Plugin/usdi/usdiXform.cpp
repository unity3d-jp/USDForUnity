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
            if (conf.swap_handedness) {
                dst.position.x *= -1.0f;
            }
            break;
        }
        case UsdGeomXformOp::TypeOrient:
        {
            if (op.GetAs((GfQuatf*)&dst.rotation, t)) { ret = true; }
            if (conf.swap_handedness) {
                auto& q = dst.rotation;
                q = { -q.x, q.y, q.z, -q.w };
            }
            const float Deg2Rad = float(M_PI) / 180.0f;
            dst.rotation *= Deg2Rad;
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
            auto& q = src.rotation;
            q = { -q.x, q.y, q.z, -q.w };
        }
        {
            const float Rad2Deg = 180.0f / float(M_PI);
            src.rotation *= Rad2Deg;
        }

        m_write_ops[0].Set((const GfVec3f&)src.position, t);
        m_write_ops[1].Set((const GfQuatf&)src.rotation, t);
        m_write_ops[2].Set((const GfVec3f&)src.scale, t);
    }
    return true;
}

} // namespace usdi
