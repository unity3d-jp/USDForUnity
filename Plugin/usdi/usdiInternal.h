#pragma once

#define usdiImpl

#define usdiLog(...) usdi::LogImpl(__VA_ARGS__)
#ifdef usdiDebug
    #define usdiTrace(...) usdi::LogImpl(__VA_ARGS__)
    #define usdiTraceFunc(...) usdi::TraceFuncImpl _trace_(__FUNCTION__)
#else
    #define usdiTrace(...)
    #define usdiTraceFunc(...)
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

extern const float Rad2Deg;
extern const float Deg2Rad;

float2 operator*(const float2& l, float r);
float3 operator*(const float3& l, float r);
float4 operator*(const float4& l, float r);
quaternion operator*(const quaternion& l, float r);
quaternion operator*(const quaternion& l, const quaternion& r);
float2& operator*=(float2& l, float r);
float3& operator*=(float3& l, float r);
float4& operator*=(float4& l, float r);
quaternion& operator*=(quaternion& l, float r);

} // namespace usdi
