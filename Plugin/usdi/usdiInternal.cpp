#include "pch.h"
#include "usdiInternal.h"

namespace usdi {

void LogImpl(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

TraceFuncImpl::TraceFuncImpl(const char *func)
    : m_func(func)
{
    usdiTrace("%s enter\n", m_func);
}

TraceFuncImpl::~TraceFuncImpl()
{
    usdiTrace("%s leave\n", m_func);
}

const float Deg2Rad = float(M_PI) / 180.0f;
const float Rad2Deg = 180.0f / float(M_PI);

float2 operator*(const float2& l, float r)
{
    return{ l.x*r, l.y*r };
}

float3 operator*(const float3& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r };
}

float4 operator*(const float4& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

quaternion operator*(const quaternion& l, float r)
{
    return{ l.x*r, l.y*r, l.z*r, l.w*r };
}

quaternion operator*(const quaternion& l, const quaternion& r)
{
    return{
        l.w*r.x + l.x*r.w + l.y*r.z - l.z*r.y,
        l.w*r.y + l.y*r.w + l.z*r.x - l.x*r.z,
        l.w*r.z + l.z*r.w + l.x*r.y - l.y*r.x,
        l.w*r.w - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}


float2& operator*=(float2& l, float r)
{
    l.x *= r;
    l.y *= r;
    return l;
}

float3& operator*=(float3& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}

float4& operator*=(float4& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}

quaternion& operator*=(quaternion& l, float r)
{
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}

} // namespace usdi
