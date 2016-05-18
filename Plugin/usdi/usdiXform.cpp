#include "pch.h"
#include "usdiInternal.h"
#include "usdiSchema.h"
#include "usdiXform.h"
#include "usdiContext.h"

namespace usdi {

static quaternion EulerToQuaternion(const float3& eular, UsdGeomXformOp::Type order)
{
    float cX = std::cos(eular.x / 2.0f);
    float sX = std::sin(eular.x / 2.0f);
    float cY = std::cos(eular.y / 2.0f);
    float sY = std::sin(eular.y / 2.0f);
    float cZ = std::cos(eular.z / 2.0f);
    float sZ = std::sin(eular.z / 2.0f);
    quaternion qX = { sX, 0.0f, 0.0f, cX };
    quaternion qY = { 0.0f, sY, 0.0f, cY };
    quaternion qZ = { 0.0f, 0.0f, sZ, cZ };

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

// return ZXY eular angles
static float3 QuaternionToEuler(const quaternion& q)
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
        case UsdGeomXformOp::TypeScale:
        {
            if (op.GetAs((GfVec3f*)&dst.scale, t)) { ret = true; }
            break;
        }
        case UsdGeomXformOp::TypeOrient:
        {
            if (op.GetAs((GfQuatf*)&dst.rotation, t)) { ret = true; }
            if (conf.swap_handedness) {
                auto& q = dst.rotation;
                q = { -q.x, q.y, q.z, -q.w };
            }
            dst.rotation *= Deg2Rad;
            break;
        }
        case UsdGeomXformOp::TypeRotateXYZ: // 
        case UsdGeomXformOp::TypeRotateXZY: // 
        case UsdGeomXformOp::TypeRotateYXZ: // 
        case UsdGeomXformOp::TypeRotateYZX: // 
        case UsdGeomXformOp::TypeRotateZXY: // 
        case UsdGeomXformOp::TypeRotateZYX: // fall through
        {
            float3 eular;
            if (op.GetAs((GfVec3f*)&eular, t)) { ret = true; }
            eular *= Deg2Rad;
            dst.rotation = EulerToQuaternion(eular, op.GetOpType());
            if (conf.swap_handedness) {
                auto& q = dst.rotation;
                q = { -q.x, q.y, q.z, -q.w };
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
    return ret;
}

//#define usdiExportRotationAsEular

bool Xform::writeSample(const XformData& src_, Time t_)
{
    auto t = (const UsdTimeCode&)t_;
    const auto& conf = getImportConfig();

    if (m_write_ops.empty()) {
        m_write_ops.push_back(m_xf.AddTranslateOp(UsdGeomXformOp::PrecisionFloat));
#ifdef usdiExportRotationAsEular
        m_write_ops.push_back(m_xf.AddRotateZXYOp(UsdGeomXformOp::PrecisionFloat));
#else // usdiExportRotationAsEular
        m_write_ops.push_back(m_xf.AddOrientOp(UsdGeomXformOp::PrecisionFloat));
#endif // usdiExportRotationAsEular
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
            src.rotation *= Rad2Deg;
        }

        m_write_ops[0].Set((const GfVec3f&)src.position, t);
#ifdef usdiExportRotationAsEular
        float3 eular = QuaternionToEuler(src.rotation) * Rad2Deg;
        m_write_ops[1].Set((const GfVec3f&)eular, t);
#else // usdiExportRotationAsEular
        m_write_ops[1].Set((const GfQuatf&)src.rotation, t);
#endif // usdiExportRotationAsEular
        m_write_ops[2].Set((const GfVec3f&)src.scale, t);
    }
    return true;
}

} // namespace usdi
