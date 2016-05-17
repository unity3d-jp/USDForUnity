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

} // namespace usdi
