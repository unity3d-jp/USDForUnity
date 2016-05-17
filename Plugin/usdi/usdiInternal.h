#pragma once

#define usdiImpl

#define usdiLog(...) usdi::LogImpl(__VA_ARGS__)
#ifdef usdiMaster
    #define usdiTrace(...)
    #define usdiTraceFunc(...)
#else
    #define usdiTrace(...) usdi::LogImpl(__VA_ARGS__)
    #define usdiTraceFunc(...) usdi::TraceFuncImpl _trace_(__FUNCTION__)
#endif

#include "usdi.h"

namespace usdi {

void LogImpl(const char *format, ...);
struct TraceFuncImpl
{
    const char *m_func;
    TraceFuncImpl(const char *func);
    ~TraceFuncImpl();
};

float2 operator*(const float2& l, float r);
float3 operator*(const float3& l, float r);
float4 operator*(const float4& l, float r);
float2& operator*=(float2& l, float r);
float3& operator*=(float3& l, float r);
float4& operator*=(float4& l, float r);

} // namespace usdi
