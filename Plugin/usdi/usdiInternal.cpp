#include "pch.h"
#include "usdiInternal.h"

namespace usdi {

int g_debug_level = 3;

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


uint32_t GetThreadID()
{
    std::hash<std::thread::id> hasher;
    return (uint32_t)hasher(std::this_thread::get_id());
}

TraceFuncImpl::TraceFuncImpl(const char *func)
    : m_func(func)
{
    usdiLogDetail("[tid %u] %s enter\n", GetThreadID(), m_func);
}

TraceFuncImpl::~TraceFuncImpl()
{
    usdiLogDetail("[tid %u] %s leave\n", GetThreadID(), m_func);
}

const float Deg2Rad = float(M_PI) / 180.0f;
const float Rad2Deg = 180.0f / float(M_PI);


#ifdef usdiDbgVTune
__itt_domain* GetVTuneDomain()
{
    static __itt_domain *s_vtune_domain = __itt_domain_create("usdi");
    s_vtune_domain->flags = 1;
    return s_vtune_domain;
}

VTuneTask::VTuneTask(const char *label)
{
    m_name = __itt_string_handle_create(label);
}

VTuneTask::~VTuneTask()
{
}

void VTuneTask::begin()
{
    __itt_task_begin(GetVTuneDomain(), __itt_null, __itt_null, m_name);
}

void VTuneTask::end()
{
    __itt_task_end(GetVTuneDomain());
}
#endif

} // namespace usdi
