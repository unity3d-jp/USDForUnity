#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiContext.h"

//#define usdiSerializeRotationAsEuler

namespace usdi {

static quatf EulerToQuaternion(const float3& euler, UsdGeomXformOp::Type order)
{
    float cX = std::cos(euler.x / 2.0f);
    float sX = std::sin(euler.x / 2.0f);
    float cY = std::cos(euler.y / 2.0f);
    float sY = std::sin(euler.y / 2.0f);
    float cZ = std::cos(euler.z / 2.0f);
    float sZ = std::sin(euler.z / 2.0f);
    quatf qX = { sX, 0.0f, 0.0f, cX };
    quatf qY = { 0.0f, sY, 0.0f, cY };
    quatf qZ = { 0.0f, 0.0f, sZ, cZ };

    switch (order) {
    case UsdGeomXformOp::TypeRotateXYZ: return (qZ * qY) * qX;
    case UsdGeomXformOp::TypeRotateXZY: return (qY * qZ) * qX;
    case UsdGeomXformOp::TypeRotateYXZ: return (qZ * qX) * qY;
    case UsdGeomXformOp::TypeRotateYZX: return (qX * qZ) * qY;
    case UsdGeomXformOp::TypeRotateZXY: return (qY * qX) * qZ;
    case UsdGeomXformOp::TypeRotateZYX: return (qX * qY) * qZ;
    }
    return {0.0f, 0.0f, 0.0f, 1.0f};
}

static float Clamp(float v, float vmin, float vmax) { return std::min<float>(std::max<float>(v, vmin), vmax); }
static float Saturate(float v) { return Clamp(v, -1.0f, 1.0f); }

static float3 QuaternionToEulerZXY(const quatf& q)
{
    float d[] = {
        q.x*q.x, q.x*q.y, q.x*q.z, q.x*q.w,
        q.y*q.y, q.y*q.z, q.y*q.w,
        q.z*q.z, q.z*q.w,
        q.w*q.w
    };

    float v0 = d[5] - d[3];
    float v1 = 2.0f*(d[1] + d[8]);
    float v2 = d[4] - d[7] - d[0] + d[9];
    float v3 = -1.0f;
    float v4 = 2.0f * v0;

    const float SINGULARITY_CUTOFF = 0.499999f;
    if (std::abs(v0) < SINGULARITY_CUTOFF)
    {
        float v5 = 2.0f * (d[2] + d[6]);
        float v6 = d[7] - d[0] - d[4] + d[9];

        return{
            v3 * std::asin(Saturate(v4)),
            std::atan2(v5, v6),
            std::atan2(v1, v2)
        };
    }
    else //x == yzy z == 0
    {
        float a = d[1] + d[8];
        float b =-d[5] + d[3];
        float c = d[1] - d[8];
        float e = d[5] + d[3];

        float v5 = a*e + b*c;
        float v6 = b*e - a*c;

        return{
            v3 * std::asin(Saturate(v4)),
            std::atan2(v5, v6),
            0.0f
        };
    }
}

static void SwapHandedness(quatf& q)
{
    q = {q.x, -q.y, -q.z, q.w};
}


Xform::Xform(Context *ctx, Schema *parent, const UsdGeomXformable& xf)
    : super(ctx, parent, xf)
    , m_xf(xf)
{
    usdiLogTrace("Xform::Xform(): %s\n", getPath());
    getTimeRange(m_summary.start, m_summary.end);
}

Xform::Xform(Context *ctx, Schema *parent, const char *name, const char *type)
    : super(ctx, parent, name, type)
    , m_xf(m_prim)
{
    usdiLogTrace("Xform::Xform(): %s\n", getPath());
}

Xform::~Xform()
{
    usdiLogTrace("Xform::~Xform(): %s\n", getPath());
}

UsdGeomXformable& Xform::getUSDSchema()
{
    return m_xf;
}

const XformSummary& Xform::getSummary() const
{
    return m_summary;
}

void Xform::updateSample(Time t_)
{
    if (!needsUpdate(t_)) {
        m_sample.flags = (m_sample.flags & ~(int)XformData::Flags::UpdatedMask);
        return;
    }
    super::updateSample(t_);

    auto t = UsdTimeCode(t_);
    const auto& conf = getImportConfig();
    auto prev = m_sample;
    auto& dst = m_sample;

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
        case UsdGeomXformOp::TypeScale:
        {
            if (op.GetAs((GfVec3f*)&dst.scale, t)) { ret = true; }
            break;
        }
        case UsdGeomXformOp::TypeOrient:
        {
            if (op.GetAs((GfQuatf*)&dst.rotation, t)) { ret = true; }
            if (conf.swap_handedness) {
                SwapHandedness(dst.rotation);
            }
            break;
        }
        case UsdGeomXformOp::TypeRotateXYZ: // 
        case UsdGeomXformOp::TypeRotateXZY: // 
        case UsdGeomXformOp::TypeRotateYXZ: // 
        case UsdGeomXformOp::TypeRotateYZX: // 
        case UsdGeomXformOp::TypeRotateZXY: // 
        case UsdGeomXformOp::TypeRotateZYX: // fall through
        {
            float3 euler;
            if (op.GetAs((GfVec3f*)&euler, t)) { ret = true; }
            dst.rotation = EulerToQuaternion(euler * Deg2Rad, op.GetOpType());
            if (conf.swap_handedness) {
                SwapHandedness(dst.rotation);
            }
            break;
        }
        case UsdGeomXformOp::TypeTransform:
        {
            // todo
            break;
        }
        }
    }

    int update_flags = 0;
    if (!NearEqual(prev.position, dst.position)) {
        update_flags |= (int)XformData::Flags::UpdatedPosition;
    }
    if (!NearEqual(prev.rotation, dst.rotation)) {
        update_flags |= (int)XformData::Flags::UpdatedRotation;
    }
    if (!NearEqual(prev.scale, dst.scale)) {
        update_flags |= (int)XformData::Flags::UpdatedScale;
    }
    dst.flags = (dst.flags & ~(int)XformData::Flags::UpdatedMask) | update_flags;
}

bool Xform::readSample(XformData& dst, Time t)
{
    updateSample(t);
    dst = m_sample;
    return true;
}


bool Xform::writeSample(const XformData& src_, Time t_)
{
    auto t = UsdTimeCode(t_);
    const auto& conf = getExportConfig();
    XformData src = src_;

    if (m_write_ops.empty()) {
        m_write_ops.push_back(m_xf.AddTranslateOp(UsdGeomXformOp::PrecisionFloat));
#ifdef usdiSerializeRotationAsEuler
        m_write_ops.push_back(m_xf.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat));
#else // usdiSerializeRotationAsEuler
        m_write_ops.push_back(m_xf.AddOrientOp(UsdGeomXformOp::PrecisionFloat));
#endif // usdiSerializeRotationAsEuler
        m_write_ops.push_back(m_xf.AddScaleOp(UsdGeomXformOp::PrecisionFloat));
    }

    if (conf.swap_handedness) {
        src.position.x *= -1.0f;
        SwapHandedness(src.rotation);
    }

    m_write_ops[0].Set((const GfVec3f&)src.position, t);

#ifdef usdiSerializeRotationAsEuler
    {
        float3 euler = QuaternionToEulerZXY(src.rotation) * Rad2Deg;
        m_write_ops[1].Set((const GfVec3f&)euler, t);
    }
#else // usdiSerializeRotationAsEuler
    {
        m_write_ops[1].Set((const GfQuatf&)src.rotation, t);
    }
#endif // usdiSerializeRotationAsEuler

    m_write_ops[2].Set((const GfVec3f&)src.scale, t);
    return true;
}

} // namespace usdi
