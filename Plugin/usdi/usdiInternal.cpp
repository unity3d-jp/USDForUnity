#include "pch.h"
#include "usdiInternal.h"

namespace usdi {

int g_debug_level = 5;

void LogImpl(int level, const char *format, ...)
{
    if (level > g_debug_level) { return; }

    va_list args;
    va_start(args, format);
#ifdef _WIN32
    const int MaxBuf = 4096;
    char buf[MaxBuf];
    vsnprintf(buf, sizeof(buf), format, args);
    ::OutputDebugStringA(buf);
#else
    vprintf(format, args);
#endif
    va_end(args);
    fflush(stdout);
}

TraceFuncImpl::TraceFuncImpl(const char *func)
    : m_func(func)
{
    usdiLogTrace("%s enter\n", m_func);
}

TraceFuncImpl::~TraceFuncImpl()
{
    usdiLogTrace("%s leave\n", m_func);
}

const float Deg2Rad = float(M_PI) / 180.0f;
const float Rad2Deg = 180.0f / float(M_PI);

} // namespace usdi
