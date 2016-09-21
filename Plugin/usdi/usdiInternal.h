#pragma once

#define usdiImpl

#define usdiLogError(...)       usdi::LogImpl(1, "usdi error: " __VA_ARGS__)
#define usdiLogWarning(...)     usdi::LogImpl(2, "usdi warning: " __VA_ARGS__)
#define usdiLogInfo(...)        usdi::LogImpl(3, "usdi info: " __VA_ARGS__)
#ifdef usdiDebug
    #define usdiLogTrace(...)   usdi::LogImpl(4, "usdi trace: " __VA_ARGS__)
    #define usdiLogDetail(...)  usdi::LogImpl(5, "usdi trace: " __VA_ARGS__)
    #define usdiTraceFunc(...)  usdi::TraceFuncImpl _trace_(__FUNCTION__)
#else
    #define usdiLogTrace(...)
    #define usdiLogDetail(...)
    #define usdiTraceFunc(...)
#endif

#include "usdi.h"

namespace usdi {

void LogImpl(int level, const char *format, ...);
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
quatf operator*(const quatf& l, float r);
quatf operator*(const quatf& l, const quatf& r);
float2& operator*=(float2& l, float r);
float3& operator*=(float3& l, float r);
float4& operator*=(float4& l, float r);
quatf& operator*=(quatf& l, float r);

} // namespace usdi

#include "usdiInternal.i"
