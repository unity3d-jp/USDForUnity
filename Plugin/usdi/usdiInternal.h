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

namespace usdi {

void LogImpl(const char *format, ...);
struct TraceFuncImpl
{
    const char *m_func;
    TraceFuncImpl(const char *func);
    ~TraceFuncImpl();
};

} // namespace usdi

#include "usdi.h"
